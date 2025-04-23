/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include "common/cs_file.h"
#include "common/cs_varint.h"

#include "mjs_array.h"
#include "mjs_bcode.h"
#include "mjs_core.h"
#include "mjs_exec.h"
#include "mjs_internal.h"
#include "mjs_object.h"
#include "mjs_parser.h"
#include "mjs_primitive.h"
#include "mjs_string.h"
#include "mjs_tok.h"
#include "mjs_util.h"
#include "mjs_array_buf.h"

#if MJS_GENERATE_JSC && defined(CS_MMAP)
#include <sys/mman.h>
#endif

/*
 * Pushes call stack frame. Offset is a global bcode offset. Retval_stack_idx
 * is an index in mjs->stack at which return value should be written later.
 */
static void call_stack_push_frame(struct mjs* mjs, size_t offset, mjs_val_t retval_stack_idx) {
    /* Pop `this` value, and apply it */
    mjs_val_t this_obj = mjs_pop_val(&mjs->arg_stack);

    /*
   * NOTE: the layout is described by enum mjs_call_stack_frame_item
   */
    push_mjs_val(&mjs->call_stack, mjs->vals.this_obj);
    mjs->vals.this_obj = this_obj;

    push_mjs_val(&mjs->call_stack, mjs_mk_number(mjs, (double)offset));
    push_mjs_val(&mjs->call_stack, mjs_mk_number(mjs, (double)mjs_stack_size(&mjs->scopes)));
    push_mjs_val(
        &mjs->call_stack, mjs_mk_number(mjs, (double)mjs_stack_size(&mjs->loop_addresses)));
    push_mjs_val(&mjs->call_stack, retval_stack_idx);
}

/*
 * Restores call stack frame. Returns the return address.
 */
static size_t call_stack_restore_frame(struct mjs* mjs) {
    size_t retval_stack_idx, return_address, scope_index, loop_addr_index;
    assert(mjs_stack_size(&mjs->call_stack) >= CALL_STACK_FRAME_ITEMS_CNT);

    /*
   * NOTE: the layout is described by enum mjs_call_stack_frame_item
   */
    retval_stack_idx = mjs_get_int(mjs, mjs_pop_val(&mjs->call_stack));
    loop_addr_index = mjs_get_int(mjs, mjs_pop_val(&mjs->call_stack));
    scope_index = mjs_get_int(mjs, mjs_pop_val(&mjs->call_stack));
    return_address = mjs_get_int(mjs, mjs_pop_val(&mjs->call_stack));
    mjs->vals.this_obj = mjs_pop_val(&mjs->call_stack);

    /* Remove created scopes */
    while(mjs_stack_size(&mjs->scopes) > scope_index) {
        mjs_pop_val(&mjs->scopes);
    }

    /* Remove loop addresses */
    while(mjs_stack_size(&mjs->loop_addresses) > loop_addr_index) {
        mjs_pop_val(&mjs->loop_addresses);
    }

    /* Shrink stack, leave return value on top */
    mjs->stack.len = retval_stack_idx * sizeof(mjs_val_t);

    /* Jump to the return address */
    return return_address;
}

static mjs_val_t mjs_find_scope(struct mjs* mjs, mjs_val_t key) {
    size_t num_scopes = mjs_stack_size(&mjs->scopes);
    while(num_scopes > 0) {
        mjs_val_t scope = *vptr(&mjs->scopes, num_scopes - 1);
        num_scopes--;
        if(mjs_get_own_property_v(mjs, scope, key) != NULL) return scope;
    }
    mjs_set_errorf(mjs, MJS_REFERENCE_ERROR, "[%s] is not defined", mjs_get_cstring(mjs, &key));
    return MJS_UNDEFINED;
}

mjs_val_t mjs_get_this(struct mjs* mjs) {
    return mjs->vals.this_obj;
}

static double do_arith_op(double da, double db, int op, bool* resnan) {
    *resnan = false;

    if(isnan(da) || isnan(db)) {
        *resnan = true;
        return 0;
    }
    /* clang-format off */
  switch (op) {
    case TOK_MINUS:   return da - db;
    case TOK_PLUS:    return da + db;
    case TOK_MUL:     return da * db;
    case TOK_DIV:
      if (db != 0) {
        return da / db;
      } else {
        /* TODO(dfrank): add support for Infinity and return it here */
        *resnan = true;
        return 0;
      }
    case TOK_REM:
      /*
       * TODO(dfrank): probably support remainder operation as it is in JS
       * (which works with non-integer divisor).
       */
      db = (int) db;
      if (db != 0) {
        bool neg = false;
        if (da < 0) {
          neg = true;
          da = -da;
        }
        if (db < 0) {
          db = -db;
        }
        da = (double) ((int64_t) da % (int64_t) db);
        if (neg) {
          da = -da;
        }
        return da;
      } else {
        *resnan = true;
        return 0;
      }
    case TOK_AND:     return (double) ((int64_t) da & (int64_t) db);
    case TOK_OR:      return (double) ((int64_t) da | (int64_t) db);
    case TOK_XOR:     return (double) ((int64_t) da ^ (int64_t) db);
    case TOK_LSHIFT:  return (double) ((int64_t) da << (int64_t) db);
    case TOK_RSHIFT:  return (double) ((int64_t) da >> (int64_t) db);
    case TOK_URSHIFT: return (double) ((uint32_t) da >> (uint32_t) db);
  }
    /* clang-format on */
    *resnan = true;
    return 0;
}

static void set_no_autoconversion_error(struct mjs* mjs) {
    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "implicit type conversion is prohibited");
}

static mjs_val_t do_op(struct mjs* mjs, mjs_val_t a, mjs_val_t b, int op) {
    mjs_val_t ret = MJS_UNDEFINED;
    bool resnan = false;
    if((mjs_is_foreign(a) || mjs_is_number(a)) && (mjs_is_foreign(b) || mjs_is_number(b))) {
        int is_result_ptr = 0;
        double da, db, result;

        if(mjs_is_foreign(a) && mjs_is_foreign(b)) {
            /* When two operands are pointers, only subtraction is supported */
            if(op != TOK_MINUS) {
                mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "invalid operands");
            }
        } else if(mjs_is_foreign(a) || mjs_is_foreign(b)) {
            /*
       * When one of the operands is a pointer, only + and - are supported,
       * and the result is a pointer.
       */
            if(op != TOK_MINUS && op != TOK_PLUS) {
                mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "invalid operands");
            }
            is_result_ptr = 1;
        }
        da = mjs_is_number(a) ? mjs_get_double(mjs, a) : (double)(uintptr_t)mjs_get_ptr(mjs, a);
        db = mjs_is_number(b) ? mjs_get_double(mjs, b) : (double)(uintptr_t)mjs_get_ptr(mjs, b);
        result = do_arith_op(da, db, op, &resnan);
        if(resnan) {
            ret = MJS_TAG_NAN;
        } else {
            /*
       * If at least one of the operands was a pointer, result should also be
       * a pointer
       */
            ret = is_result_ptr ? mjs_mk_foreign(mjs, (void*)(uintptr_t)result) :
                                  mjs_mk_number(mjs, result);
        }
    } else if(mjs_is_string(a) && mjs_is_string(b) && (op == TOK_PLUS)) {
        ret = s_concat(mjs, a, b);
    } else {
        set_no_autoconversion_error(mjs);
    }
    return ret;
}

static void op_assign(struct mjs* mjs, int op) {
    mjs_val_t val = mjs_pop(mjs);
    mjs_val_t obj = mjs_pop(mjs);
    mjs_val_t key = mjs_pop(mjs);
    if(mjs_is_object(obj) && mjs_is_string(key)) {
        mjs_val_t v = mjs_get_v(mjs, obj, key);
        mjs_set_v(mjs, obj, key, do_op(mjs, v, val, op));
        mjs_push(mjs, v);
    } else {
        mjs_set_errorf(mjs, MJS_TYPE_ERROR, "invalid operand");
    }
}

static int check_equal(struct mjs* mjs, mjs_val_t a, mjs_val_t b) {
    int ret = 0;
    if(a == MJS_TAG_NAN && b == MJS_TAG_NAN) {
        ret = 0;
    } else if(a == b) {
        ret = 1;
    } else if(mjs_is_number(a) && mjs_is_number(b)) {
        /*
     * The case of equal numbers is handled above, so here the result is always
     * false
     */
        ret = 0;
    } else if(mjs_is_string(a) && mjs_is_string(b)) {
        ret = s_cmp(mjs, a, b) == 0;
    } else if(mjs_is_foreign(a) && b == MJS_NULL) {
        ret = mjs_get_ptr(mjs, a) == NULL;
    } else if(a == MJS_NULL && mjs_is_foreign(b)) {
        ret = mjs_get_ptr(mjs, b) == NULL;
    } else {
        ret = 0;
    }
    return ret;
}

static void exec_expr(struct mjs* mjs, int op) {
    switch(op) {
    case TOK_DOT:
        break;
    case TOK_MINUS:
    case TOK_PLUS:
    case TOK_MUL:
    case TOK_DIV:
    case TOK_REM:
    case TOK_XOR:
    case TOK_AND:
    case TOK_OR:
    case TOK_LSHIFT:
    case TOK_RSHIFT:
    case TOK_URSHIFT: {
        mjs_val_t b = mjs_pop(mjs);
        mjs_val_t a = mjs_pop(mjs);
        mjs_push(mjs, do_op(mjs, a, b, op));
        break;
    }
    case TOK_UNARY_MINUS: {
        double a = mjs_get_double(mjs, mjs_pop(mjs));
        mjs_push(mjs, mjs_mk_number(mjs, -a));
        break;
    }
    case TOK_NOT: {
        mjs_val_t val = mjs_pop(mjs);
        mjs_push(mjs, mjs_mk_boolean(mjs, !mjs_is_truthy(mjs, val)));
        break;
    }
    case TOK_TILDA: {
        double a = mjs_get_double(mjs, mjs_pop(mjs));
        mjs_push(mjs, mjs_mk_number(mjs, (double)(~(int64_t)a)));
        break;
    }
    case TOK_UNARY_PLUS:
        break;
    case TOK_EQ:
        mjs_set_errorf(mjs, MJS_NOT_IMPLEMENTED_ERROR, "Use ===, not ==");
        break;
    case TOK_NE:
        mjs_set_errorf(mjs, MJS_NOT_IMPLEMENTED_ERROR, "Use !==, not !=");
        break;
    case TOK_EQ_EQ: {
        mjs_val_t a = mjs_pop(mjs);
        mjs_val_t b = mjs_pop(mjs);
        mjs_push(mjs, mjs_mk_boolean(mjs, check_equal(mjs, a, b)));
        break;
    }
    case TOK_NE_NE: {
        mjs_val_t a = mjs_pop(mjs);
        mjs_val_t b = mjs_pop(mjs);
        mjs_push(mjs, mjs_mk_boolean(mjs, !check_equal(mjs, a, b)));
        break;
    }
    case TOK_LT: {
        double b = mjs_get_double(mjs, mjs_pop(mjs));
        double a = mjs_get_double(mjs, mjs_pop(mjs));
        mjs_push(mjs, mjs_mk_boolean(mjs, a < b));
        break;
    }
    case TOK_GT: {
        double b = mjs_get_double(mjs, mjs_pop(mjs));
        double a = mjs_get_double(mjs, mjs_pop(mjs));
        mjs_push(mjs, mjs_mk_boolean(mjs, a > b));
        break;
    }
    case TOK_LE: {
        double b = mjs_get_double(mjs, mjs_pop(mjs));
        double a = mjs_get_double(mjs, mjs_pop(mjs));
        mjs_push(mjs, mjs_mk_boolean(mjs, a <= b));
        break;
    }
    case TOK_GE: {
        double b = mjs_get_double(mjs, mjs_pop(mjs));
        double a = mjs_get_double(mjs, mjs_pop(mjs));
        mjs_push(mjs, mjs_mk_boolean(mjs, a >= b));
        break;
    }
    case TOK_ASSIGN: {
        mjs_val_t val = mjs_pop(mjs);
        mjs_val_t obj = mjs_pop(mjs);
        mjs_val_t key = mjs_pop(mjs);
        if(mjs_is_object(obj)) {
            mjs_set_v(mjs, obj, key, val);
        } else if(mjs_is_data_view(obj)) {
            mjs_err_t err = mjs_dataview_set_prop(mjs, obj, key, val);
            if(err != MJS_OK) {
                mjs_prepend_errorf(mjs, err, "");
            }
        } else if(mjs_is_foreign(obj)) {
            /*
         * We don't have setters, so in order to support properties which behave
         * like setters, we have to parse key right here, instead of having real
         * built-in prototype objects
         */

            int ikey = mjs_get_int(mjs, key);
            int ival = mjs_get_int(mjs, val);

            if(!mjs_is_number(key)) {
                mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "index must be a number");
                val = MJS_UNDEFINED;
            } else if(!mjs_is_number(val) || ival < 0 || ival > 0xff) {
                mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "only number 0 .. 255 can be assigned");
                val = MJS_UNDEFINED;
            } else {
                uint8_t* ptr = (uint8_t*)mjs_get_ptr(mjs, obj);
                *(ptr + ikey) = (uint8_t)ival;
            }
        } else {
            mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "unsupported object type");
        }
        mjs_push(mjs, val);
        break;
    }
    case TOK_POSTFIX_PLUS: {
        mjs_val_t obj = mjs_pop(mjs);
        mjs_val_t key = mjs_pop(mjs);
        if(mjs_is_object(obj) && mjs_is_string(key)) {
            mjs_val_t v = mjs_get_v(mjs, obj, key);
            mjs_val_t v1 = do_op(mjs, v, mjs_mk_number(mjs, 1), TOK_PLUS);
            mjs_set_v(mjs, obj, key, v1);
            mjs_push(mjs, v);
        } else {
            mjs_set_errorf(mjs, MJS_TYPE_ERROR, "invalid operand for ++");
        }
        break;
    }
    case TOK_POSTFIX_MINUS: {
        mjs_val_t obj = mjs_pop(mjs);
        mjs_val_t key = mjs_pop(mjs);
        if(mjs_is_object(obj) && mjs_is_string(key)) {
            mjs_val_t v = mjs_get_v(mjs, obj, key);
            mjs_val_t v1 = do_op(mjs, v, mjs_mk_number(mjs, 1), TOK_MINUS);
            mjs_set_v(mjs, obj, key, v1);
            mjs_push(mjs, v);
        } else {
            mjs_set_errorf(mjs, MJS_TYPE_ERROR, "invalid operand for --");
        }
        break;
    }
    case TOK_MINUS_MINUS: {
        mjs_val_t obj = mjs_pop(mjs);
        mjs_val_t key = mjs_pop(mjs);
        if(mjs_is_object(obj) && mjs_is_string(key)) {
            mjs_val_t v = mjs_get_v(mjs, obj, key);
            v = do_op(mjs, v, mjs_mk_number(mjs, 1), TOK_MINUS);
            mjs_set_v(mjs, obj, key, v);
            mjs_push(mjs, v);
        } else {
            mjs_set_errorf(mjs, MJS_TYPE_ERROR, "invalid operand for --");
        }
        break;
    }
    case TOK_PLUS_PLUS: {
        mjs_val_t obj = mjs_pop(mjs);
        mjs_val_t key = mjs_pop(mjs);
        if(mjs_is_object(obj) && mjs_is_string(key)) {
            mjs_val_t v = mjs_get_v(mjs, obj, key);
            v = do_op(mjs, v, mjs_mk_number(mjs, 1), TOK_PLUS);
            mjs_set_v(mjs, obj, key, v);
            mjs_push(mjs, v);
        } else {
            mjs_set_errorf(mjs, MJS_TYPE_ERROR, "invalid operand for ++");
        }
        break;
    }
        /*
     * NOTE: TOK_LOGICAL_AND and TOK_LOGICAL_OR don't need to be here, because
     * they are just naturally handled by the short-circuit evaluation.
     * See PARSE_LTR_BINOP() macro in mjs_parser.c.
     */

        /* clang-format off */
    case TOK_MINUS_ASSIGN:    op_assign(mjs, TOK_MINUS);    break;
    case TOK_PLUS_ASSIGN:     op_assign(mjs, TOK_PLUS);     break;
    case TOK_MUL_ASSIGN:      op_assign(mjs, TOK_MUL);      break;
    case TOK_DIV_ASSIGN:      op_assign(mjs, TOK_DIV);      break;
    case TOK_REM_ASSIGN:      op_assign(mjs, TOK_REM);      break;
    case TOK_AND_ASSIGN:      op_assign(mjs, TOK_AND);      break;
    case TOK_OR_ASSIGN:       op_assign(mjs, TOK_OR);       break;
    case TOK_XOR_ASSIGN:      op_assign(mjs, TOK_XOR);      break;
    case TOK_LSHIFT_ASSIGN:   op_assign(mjs, TOK_LSHIFT);   break;
    case TOK_RSHIFT_ASSIGN:   op_assign(mjs, TOK_RSHIFT);   break;
    case TOK_URSHIFT_ASSIGN:  op_assign(mjs, TOK_URSHIFT);  break;
    case TOK_COMMA: break;
    /* clang-format on */
    case TOK_KEYWORD_TYPEOF:
        mjs_push(mjs, mjs_mk_string(mjs, mjs_typeof(mjs_pop(mjs)), ~0, 1));
        break;
    default:
        LOG(LL_ERROR, ("Unknown expr: %d", op));
        break;
    }
}

static int getprop_builtin_string(
    struct mjs* mjs,
    mjs_val_t val,
    const char* name,
    size_t name_len,
    mjs_val_t* res) {
    int isnum = 0;
    int idx = cstr_to_ulong(name, name_len, &isnum);

    if(strcmp(name, "length") == 0) {
        size_t val_len;
        mjs_get_string(mjs, &val, &val_len);
        *res = mjs_mk_number(mjs, (double)val_len);
        return 1;
    } else if(strcmp(name, "at") == 0 || strcmp(name, "charCodeAt") == 0) {
        *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_string_char_code_at);
        return 1;
    } else if(strcmp(name, "indexOf") == 0) {
        *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_string_index_of);
        return 1;
    } else if(strcmp(name, "slice") == 0) {
        *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_string_slice);
        return 1;
    } else if(strcmp(name, "toUpperCase") == 0) {
        *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_string_to_upper_case);
        return 1;
    } else if(strcmp(name, "toLowerCase") == 0) {
        *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_string_to_lower_case);
        return 1;
    } else if(isnum) {
        /*
     * string subscript: return a new one-byte string if the index
     * is not out of bounds
     */
        size_t val_len;
        const char* str = mjs_get_string(mjs, &val, &val_len);
        if(idx >= 0 && idx < (int)val_len) {
            *res = mjs_mk_string(mjs, str + idx, 1, 1);
        } else {
            *res = MJS_UNDEFINED;
        }
        return 1;
    }
    return 0;
}

static int getprop_builtin_number(
    struct mjs* mjs,
    mjs_val_t val,
    const char* name,
    size_t name_len,
    mjs_val_t* res) {
    if(strcmp(name, "toString") == 0) {
        *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_number_to_string);
        return 1;
    }

    (void)val;
    (void)name_len;
    return 0;
}

static int getprop_builtin_array(
    struct mjs* mjs,
    mjs_val_t val,
    const char* name,
    size_t name_len,
    mjs_val_t* res) {
    if(strcmp(name, "splice") == 0) {
        *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_array_splice);
        return 1;
    } else if(strcmp(name, "push") == 0) {
        *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_array_push_internal);
        return 1;
    } else if(strcmp(name, "length") == 0) {
        *res = mjs_mk_number(mjs, mjs_array_length(mjs, val));
        return 1;
    }

    (void)name_len;
    return 0;
}

static int getprop_builtin_foreign(
    struct mjs* mjs,
    mjs_val_t val,
    const char* name,
    size_t name_len,
    mjs_val_t* res) {
    int isnum = 0;
    int idx = cstr_to_ulong(name, name_len, &isnum);

    if(!isnum) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "index must be a number");
    } else {
        uint8_t* ptr = (uint8_t*)mjs_get_ptr(mjs, val);
        *res = mjs_mk_number(mjs, *(ptr + idx));
    }
    return 1;
}

static int getprop_builtin_array_buf(
    struct mjs* mjs,
    mjs_val_t val,
    const char* name,
    size_t name_len,
    mjs_val_t* res) {
    if(strcmp(name, "byteLength") == 0) {
        size_t len = 0;
        mjs_array_buf_get_ptr(mjs, val, &len);
        *res = mjs_mk_number(mjs, len);
        return 1;
    } else if(strcmp(name, "getPtr") == 0) {
        void* ptr = mjs_array_buf_get_ptr(mjs, val, NULL);
        *res = mjs_mk_foreign(mjs, ptr);
        return 1;
    } else if(strcmp(name, "slice") == 0) {
        *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_array_buf_slice);
        return 1;
    }

    (void)name_len;
    return 0;
}

static int getprop_builtin_data_view(
    struct mjs* mjs,
    mjs_val_t val,
    const char* name,
    size_t name_len,
    mjs_val_t* res) {
    if(strcmp(name, "byteLength") == 0) {
        size_t len = 0;
        mjs_array_buf_get_ptr(mjs, mjs_dataview_get_buf(mjs, val), &len);
        *res = mjs_mk_number(mjs, len);
        return 1;
    } else if(strcmp(name, "length") == 0) {
        *res = mjs_dataview_get_len(mjs, val);
        return 1;
    } else if(strcmp(name, "buffer") == 0) {
        *res = mjs_dataview_get_buf(mjs, val);
        return 1;
    }

    (void)name_len;
    return 0;
}

static void mjs_apply_(struct mjs* mjs) {
    mjs_val_t res = MJS_UNDEFINED, *args = NULL;
    mjs_val_t func = mjs->vals.this_obj, v = mjs_arg(mjs, 1);
    int i, nargs = 0;
    if(mjs_is_array(v)) {
        nargs = mjs_array_length(mjs, v);
        args = calloc(nargs, sizeof(args[0]));
        for(i = 0; i < nargs; i++)
            args[i] = mjs_array_get(mjs, v, i);
    }
    mjs_apply(mjs, &res, func, mjs_arg(mjs, 0), nargs, args);
    free(args);
    mjs_return(mjs, res);
}

static int getprop_builtin(struct mjs* mjs, mjs_val_t val, mjs_val_t name, mjs_val_t* res) {
    size_t n;
    char* s = NULL;
    int need_free = 0;
    int handled = 0;

    mjs_err_t err = mjs_to_string(mjs, &name, &s, &n, &need_free);

    if(err == MJS_OK) {
        if(mjs_is_string(val)) {
            handled = getprop_builtin_string(mjs, val, s, n, res);
        } else if(s != NULL && n == 5 && strncmp(s, "apply", n) == 0) {
            *res = mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_apply_);
            handled = 1;
        } else if(mjs_is_number(val)) {
            handled = getprop_builtin_number(mjs, val, s, n, res);
        } else if(mjs_is_array(val)) {
            handled = getprop_builtin_array(mjs, val, s, n, res);
        } else if(mjs_is_foreign(val)) {
            handled = getprop_builtin_foreign(mjs, val, s, n, res);
        } else if(mjs_is_array_buf(val)) {
            handled = getprop_builtin_array_buf(mjs, val, s, n, res);
        } else if(mjs_is_data_view(val)) {
            handled = getprop_builtin_data_view(mjs, val, s, n, res);
        }
    }

    if(need_free) {
        free(s);
        s = NULL;
    }

    return handled;
}

MJS_PRIVATE mjs_err_t mjs_execute(struct mjs* mjs, size_t off, mjs_val_t* res) {
    size_t i;
    uint8_t prev_opcode = OP_MAX;
    uint8_t opcode = OP_MAX;

    /*
   * remember lengths of all stacks, they will be restored in case of an error
   */
    int stack_len = mjs->stack.len;
    int call_stack_len = mjs->call_stack.len;
    int arg_stack_len = mjs->arg_stack.len;
    int scopes_len = mjs->scopes.len;
    int loop_addresses_len = mjs->loop_addresses.len;
    size_t start_off = off;
    const uint8_t* code;

    struct mjs_bcode_part bp = *mjs_bcode_part_get_by_offset(mjs, off);

    mjs_set_errorf(mjs, MJS_OK, NULL);
    free(mjs->stack_trace);
    mjs->stack_trace = NULL;

    off -= bp.start_idx;

    for(i = off; i < bp.data.len; i++) {
        mjs->cur_bcode_offset = i;

        if(mjs->need_gc) {
            if(maybe_gc(mjs)) {
                mjs->need_gc = 0;
            }
        }
#if MJS_AGGRESSIVE_GC
        maybe_gc(mjs);
#endif

        code = (const uint8_t*)bp.data.p;
#if MJS_ENABLE_DEBUG
        mjs_disasm_single(code, i);
#endif
        prev_opcode = opcode;
        opcode = code[i];
        switch(opcode) {
        case OP_BCODE_HEADER: {
            mjs_header_item_t bcode_offset;
            memcpy(
                &bcode_offset,
                code + i + 1 + sizeof(mjs_header_item_t) * MJS_HDR_ITEM_BCODE_OFFSET,
                sizeof(bcode_offset));
            i += bcode_offset;
        } break;
        case OP_PUSH_NULL:
            mjs_push(mjs, mjs_mk_null());
            break;
        case OP_PUSH_UNDEF:
            mjs_push(mjs, mjs_mk_undefined());
            break;
        case OP_PUSH_FALSE:
            mjs_push(mjs, mjs_mk_boolean(mjs, 0));
            break;
        case OP_PUSH_TRUE:
            mjs_push(mjs, mjs_mk_boolean(mjs, 1));
            break;
        case OP_PUSH_OBJ:
            mjs_push(mjs, mjs_mk_object(mjs));
            break;
        case OP_PUSH_ARRAY:
            mjs_push(mjs, mjs_mk_array(mjs));
            break;
        case OP_PUSH_FUNC: {
            int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
            mjs_push(mjs, mjs_mk_function(mjs, bp.start_idx + i - n));
            i += llen;
            break;
        }
        case OP_PUSH_THIS:
            mjs_push(mjs, mjs->vals.this_obj);
            break;
        case OP_JMP: {
            int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
            i += n + llen;
            break;
        }
        case OP_JMP_FALSE: {
            int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
            i += llen;
            if(!mjs_is_truthy(mjs, mjs_pop(mjs))) {
                mjs_push(mjs, MJS_UNDEFINED);
                i += n;
            }
            break;
        }
        /*
       * OP_JMP_NEUTRAL_... ops are like as OP_JMP_..., but they are completely
       * stack-neutral: they just check the TOS, and increment instruction
       * pointer if the TOS is truthy/falsy.
       */
        case OP_JMP_NEUTRAL_TRUE: {
            int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
            i += llen;
            if(mjs_is_truthy(mjs, vtop(&mjs->stack))) {
                i += n;
            }
            break;
        }
        case OP_JMP_NEUTRAL_FALSE: {
            int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
            i += llen;
            if(!mjs_is_truthy(mjs, vtop(&mjs->stack))) {
                i += n;
            }
            break;
        }
        case OP_FIND_SCOPE: {
            mjs_val_t key = vtop(&mjs->stack);
            mjs_push(mjs, mjs_find_scope(mjs, key));
            break;
        }
        case OP_CREATE: {
            mjs_val_t obj = mjs_pop(mjs);
            mjs_val_t key = mjs_pop(mjs);
            if(mjs_get_own_property_v(mjs, obj, key) == NULL) {
                mjs_set_v(mjs, obj, key, MJS_UNDEFINED);
            }
            break;
        }
        case OP_APPEND: {
            mjs_val_t val = mjs_pop(mjs);
            mjs_val_t arr = mjs_pop(mjs);
            mjs_err_t err = mjs_array_push(mjs, arr, val);
            if(err != MJS_OK) {
                mjs_set_errorf(mjs, MJS_TYPE_ERROR, "append to non-array");
            }
            break;
        }
        case OP_GET: {
            mjs_val_t obj = mjs_pop(mjs);
            mjs_val_t key = mjs_pop(mjs);
            mjs_val_t val = MJS_UNDEFINED;

            if(!getprop_builtin(mjs, obj, key, &val)) {
                if(mjs_is_object(obj)) {
                    val = mjs_get_v_proto(mjs, obj, key);
                } else if((mjs_is_data_view(obj) && (mjs_is_number(key)))) {
                    val = mjs_dataview_get_prop(mjs, obj, key);
                } else {
                    mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "type error");
                }
            }

            mjs_push(mjs, val);
            if(prev_opcode != OP_FIND_SCOPE) {
                /*
           * Previous opcode was not OP_FIND_SCOPE, so it's some "custom"
           * object which might be used as `this`, so, save it
           */
                mjs->vals.last_getprop_obj = obj;
            } else {
                /*
           * Previous opcode was OP_FIND_SCOPE, so we're getting value from
           * the scope, and it should *not* be used as `this`
           */
                mjs->vals.last_getprop_obj = MJS_UNDEFINED;
            }
            break;
        }
        case OP_DEL_SCOPE:
            if(mjs->scopes.len <= 1) {
                mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "scopes underflow");
            } else {
                mjs_pop_val(&mjs->scopes);
            }
            break;
        case OP_NEW_SCOPE:
            push_mjs_val(&mjs->scopes, mjs_mk_object(mjs));
            break;
        case OP_PUSH_SCOPE:
            assert(mjs_stack_size(&mjs->scopes) > 0);
            mjs_push(mjs, vtop(&mjs->scopes));
            break;
        case OP_PUSH_STR: {
            int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
            mjs_push(mjs, mjs_mk_string(mjs, (char*)code + i + 1 + llen, n, 1));
            i += llen + n;
            break;
        }
        case OP_PUSH_INT: {
            int llen;
            int64_t n = cs_varint_decode_unsafe(&code[i + 1], &llen);
            mjs_push(mjs, mjs_mk_number(mjs, (double)n));
            i += llen;
            break;
        }
        case OP_PUSH_DBL: {
            int llen, n = cs_varint_decode_unsafe(&code[i + 1], &llen);
            mjs_push(mjs, mjs_mk_number(mjs, strtod((char*)code + i + 1 + llen, NULL)));
            i += llen + n;
            break;
        }
        case OP_FOR_IN_NEXT: {
            /*
         * Data stack layout:
         * ...                                    <-- Bottom of the data stack
         * <iterator_variable_name>   (string)
         * <object_that_is_iterated>  (object)
         * <iterator_foreign_ptr>                 <-- Top of the data stack
         */
            mjs_val_t* iterator = vptr(&mjs->stack, -1);
            mjs_val_t obj = *vptr(&mjs->stack, -2);
            if(mjs_is_object(obj)) {
                mjs_val_t var_name = *vptr(&mjs->stack, -3);
                mjs_val_t key = mjs_next(mjs, obj, iterator);
                if(key != MJS_UNDEFINED) {
                    mjs_val_t scope = mjs_find_scope(mjs, var_name);
                    mjs_set_v(mjs, scope, var_name, key);
                }
            } else {
                mjs_set_errorf(mjs, MJS_TYPE_ERROR, "can't iterate over non-object value");
            }
            break;
        }
        case OP_RETURN: {
            /*
         * Return address is saved as a global bcode offset, so we need to
         * convert it to the local offset
         */
            size_t off_ret = call_stack_restore_frame(mjs);
            if(off_ret != MJS_BCODE_OFFSET_EXIT) {
                bp = *mjs_bcode_part_get_by_offset(mjs, off_ret);
                code = (const uint8_t*)bp.data.p;
                i = off_ret - bp.start_idx;
                LOG(LL_VERBOSE_DEBUG, ("RETURNING TO %d", (int)off_ret + 1));
            } else {
                goto clean;
            }
            // mjs_dump(mjs, 0, stdout);
            break;
        }
        case OP_ARGS: {
            /*
         * If OP_ARGS follows OP_GET, then last_getprop_obj is set to `this`
         * value; otherwise, last_getprop_obj is irrelevant and we have to
         * reset it to `undefined`
         */
            if(prev_opcode != OP_GET) {
                mjs->vals.last_getprop_obj = MJS_UNDEFINED;
            }

            /*
         * Push last_getprop_obj, which is going to be used as `this`, see
         * OP_CALL
         */
            push_mjs_val(&mjs->arg_stack, mjs->vals.last_getprop_obj);
            /*
         * Push current size of data stack, it's needed to place arguments
         * properly
         */
            push_mjs_val(&mjs->arg_stack, mjs_mk_number(mjs, (double)mjs_stack_size(&mjs->stack)));
            break;
        }
        case OP_CALL: {
            // LOG(LL_INFO, ("BEFORE CALL"));
            // mjs_dump(mjs, 0, stdout);
            int func_pos;
            mjs_val_t* func;
            mjs_val_t retval_stack_idx = vtop(&mjs->arg_stack);
            func_pos = mjs_get_int(mjs, retval_stack_idx) - 1;
            func = vptr(&mjs->stack, func_pos);

            /* Drop data stack size (pushed by OP_ARGS) */
            mjs_pop_val(&mjs->arg_stack);

            if(mjs_is_function(*func)) {
                size_t off_call;
                call_stack_push_frame(mjs, bp.start_idx + i, retval_stack_idx);

                /*
           * Function offset is a global bcode offset, so we need to convert it
           * to the local offset
           */
                off_call = mjs_get_func_addr(*func) - 1;
                bp = *mjs_bcode_part_get_by_offset(mjs, off_call);
                code = (const uint8_t*)bp.data.p;
                i = off_call - bp.start_idx;

                *func = MJS_UNDEFINED; // Return value
                // LOG(LL_VERBOSE_DEBUG, ("CALLING  %d", i + 1));
            } else if(mjs_is_string(*func) || mjs_is_ffi_sig(*func)) {
                /* Call ffi-ed function */

                call_stack_push_frame(mjs, bp.start_idx + i, retval_stack_idx);

                /* Perform the ffi-ed function call */
                mjs_ffi_call2(mjs);

                call_stack_restore_frame(mjs);
            } else if(mjs_is_foreign(*func)) {
                /* Call cfunction */

                call_stack_push_frame(mjs, bp.start_idx + i, retval_stack_idx);

                /* Perform the cfunction call */
                ((void (*)(struct mjs*))mjs_get_ptr(mjs, *func))(mjs);

                call_stack_restore_frame(mjs);
            } else {
                mjs_set_errorf(mjs, MJS_TYPE_ERROR, "calling non-callable");
            }
            break;
        }
        case OP_SET_ARG: {
            int llen1, llen2, n, arg_no = cs_varint_decode_unsafe(&code[i + 1], &llen1);
            mjs_val_t obj, key, v;
            n = cs_varint_decode_unsafe(&code[i + llen1 + 1], &llen2);
            key = mjs_mk_string(mjs, (char*)code + i + 1 + llen1 + llen2, n, 1);
            obj = vtop(&mjs->scopes);
            v = mjs_arg(mjs, arg_no);
            mjs_set_v(mjs, obj, key, v);
            i += llen1 + llen2 + n;
            break;
        }
        case OP_SETRETVAL: {
            if(mjs_stack_size(&mjs->call_stack) < CALL_STACK_FRAME_ITEMS_CNT) {
                mjs_set_errorf(mjs, MJS_INTERNAL_ERROR, "cannot return");
            } else {
                size_t retval_pos = mjs_get_int(
                    mjs, *vptr(&mjs->call_stack, -1 - CALL_STACK_FRAME_ITEM_RETVAL_STACK_IDX));
                *vptr(&mjs->stack, retval_pos - 1) = mjs_pop(mjs);
            }
            // LOG(LL_INFO, ("AFTER SETRETVAL"));
            // mjs_dump(mjs, 0, stdout);
            break;
        }
        case OP_EXPR: {
            int op = code[i + 1];
            exec_expr(mjs, op);
            i++;
            break;
        }
        case OP_DROP: {
            mjs_pop(mjs);
            break;
        }
        case OP_DUP: {
            mjs_push(mjs, vtop(&mjs->stack));
            break;
        }
        case OP_SWAP: {
            mjs_val_t a = mjs_pop(mjs);
            mjs_val_t b = mjs_pop(mjs);
            mjs_push(mjs, a);
            mjs_push(mjs, b);
            break;
        }
        case OP_LOOP: {
            int l1, l2, off = cs_varint_decode_unsafe(&code[i + 1], &l1);
            /* push scope index */
            push_mjs_val(
                &mjs->loop_addresses, mjs_mk_number(mjs, (double)mjs_stack_size(&mjs->scopes)));

            /* push break offset */
            push_mjs_val(
                &mjs->loop_addresses,
                mjs_mk_number(mjs, (double)(i + 1 /* OP_LOOP */ + l1 + off)));
            off = cs_varint_decode_unsafe(&code[i + 1 + l1], &l2);

            /* push continue offset */
            push_mjs_val(
                &mjs->loop_addresses,
                mjs_mk_number(mjs, (double)(i + 1 /* OP_LOOP*/ + l1 + l2 + off)));
            i += l1 + l2;
            break;
        }
        case OP_CONTINUE: {
            if(mjs_stack_size(&mjs->loop_addresses) >= 3) {
                size_t scopes_len = mjs_get_int(mjs, *vptr(&mjs->loop_addresses, -3));
                assert(mjs_stack_size(&mjs->scopes) >= scopes_len);
                mjs->scopes.len = scopes_len * sizeof(mjs_val_t);

                /* jump to "continue" address */
                i = mjs_get_int(mjs, vtop(&mjs->loop_addresses)) - 1;
            } else {
                mjs_set_errorf(mjs, MJS_SYNTAX_ERROR, "misplaced 'continue'");
            }
        } break;
        case OP_BREAK: {
            if(mjs_stack_size(&mjs->loop_addresses) >= 3) {
                size_t scopes_len;
                /* drop "continue" address */
                mjs_pop_val(&mjs->loop_addresses);

                /* pop "break" address and jump to it */
                i = mjs_get_int(mjs, mjs_pop_val(&mjs->loop_addresses)) - 1;

                /* restore scope index */
                scopes_len = mjs_get_int(mjs, mjs_pop_val(&mjs->loop_addresses));
                assert(mjs_stack_size(&mjs->scopes) >= scopes_len);
                mjs->scopes.len = scopes_len * sizeof(mjs_val_t);

                LOG(LL_VERBOSE_DEBUG, ("BREAKING TO %d", (int)i + 1));
            } else {
                mjs_set_errorf(mjs, MJS_SYNTAX_ERROR, "misplaced 'break'");
            }
        } break;
        case OP_NOP:
            break;
        case OP_EXIT:
            i = bp.data.len;
            break;
        default:
#if MJS_ENABLE_DEBUG
            mjs_dump(mjs, 1);
#endif
            mjs_set_errorf(
                mjs,
                MJS_INTERNAL_ERROR,
                "Unknown opcode: %d, off %d+%d",
                (int)opcode,
                (int)bp.start_idx,
                (int)i);
            i = bp.data.len;
            break;
        }

        if(mjs->exec_flags_poller) {
            mjs->exec_flags_poller(mjs);
        }

        if(mjs->error != MJS_OK) {
            if(mjs->error == MJS_NEED_EXIT) {
                mjs->error = MJS_OK;
                goto clean;
            }

            mjs_gen_stack_trace(mjs, bp.start_idx + i - 1 /* undo the i++ */);

            /* restore stack lenghts */
            mjs->stack.len = stack_len;
            mjs->call_stack.len = call_stack_len;
            mjs->arg_stack.len = arg_stack_len;
            mjs->scopes.len = scopes_len;
            mjs->loop_addresses.len = loop_addresses_len;

            /* script will evaluate to `undefined` */
            mjs_push(mjs, MJS_UNDEFINED);
            break;
        }
    }

clean:
    /* Remember result of the evaluation of this bcode part */
    mjs_bcode_part_get_by_offset(mjs, start_off)->exec_res = mjs->error;

    *res = mjs_pop(mjs);
    return mjs->error;
}

MJS_PRIVATE mjs_err_t mjs_exec_internal(
    struct mjs* mjs,
    const char* path,
    const char* src,
    int generate_jsc,
    mjs_val_t* res) {
    size_t off = mjs->bcode_len;
    mjs_val_t r = MJS_UNDEFINED;
    mjs->error = mjs_parse(path, src, mjs);
#if MJS_ENABLE_DEBUG
    if(cs_log_level >= LL_VERBOSE_DEBUG) mjs_dump(mjs, 1);
#endif
    if(generate_jsc == -1) generate_jsc = mjs->generate_jsc;
    if(mjs->error == MJS_OK) {
#if MJS_GENERATE_JSC && defined(CS_MMAP)
        if(generate_jsc && path != NULL) {
            const char* jsext = ".js";
            int basename_len = (int)strlen(path) - strlen(jsext);
            if(basename_len > 0 && strcmp(path + basename_len, jsext) == 0) {
                /* source file has a .js extension: create a .jsc counterpart */
                int rewrite = 1;
                int read_mmapped = 1;

                /* construct .jsc filename */
                const char* jscext = ".jsc";
                char filename_jsc[basename_len + strlen(jscext) + 1 /* nul-term */];
                memcpy(filename_jsc, path, basename_len);
                strcpy(filename_jsc + basename_len, jscext);

                /* get last bcode part */
                struct mjs_bcode_part* bp = mjs_bcode_part_get(mjs, mjs_bcode_parts_cnt(mjs) - 1);

                /*
         * before writing .jsc file, check if it already exists and has the
         * same contents
         *
         * TODO(dfrank): probably store crc32 before the bcode data, and only
         * compare it.
         */
                {
                    size_t size;
                    char* data = cs_mmap_file(filename_jsc, &size);
                    if(data != NULL) {
                        if(size == bp->data.len) {
                            if(memcmp(data, bp->data.p, size) == 0) {
                                /* .jsc file is up to date, so don't rewrite it */
                                rewrite = 0;
                            }
                        }
                        munmap(data, size);
                    }
                }

                /* try to open .jsc file for writing */
                if(rewrite) {
                    FILE* fp = fopen(filename_jsc, "wb");
                    if(fp != NULL) {
                        /* write last bcode part to .jsc */
                        fwrite(bp->data.p, bp->data.len, 1, fp);
                        fclose(fp);
                    } else {
                        LOG(LL_WARN, ("Failed to open %s for writing", filename_jsc));
                        read_mmapped = 0;
                    }
                }

                if(read_mmapped) {
                    /* free RAM buffer with last bcode part */
                    free((void*)bp->data.p);

                    /* mmap .jsc file and set last bcode part buffer to it */
                    bp->data.p = cs_mmap_file(filename_jsc, &bp->data.len);
                    bp->in_rom = 1;
                }
            }
        }
#else
        (void)generate_jsc;
#endif

        mjs_execute(mjs, off, &r);
    }
    if(res != NULL) *res = r;
    return mjs->error;
}

mjs_err_t mjs_exec(struct mjs* mjs, const char* src, mjs_val_t* res) {
    return mjs_exec_internal(mjs, "<stdin>", src, 0 /* generate_jsc */, res);
}

mjs_err_t mjs_exec_file(struct mjs* mjs, const char* path, mjs_val_t* res) {
    mjs_err_t error = MJS_FILE_READ_ERROR;
    mjs_val_t r = MJS_UNDEFINED;
    size_t size;
    char* source_code = cs_read_file(path, &size);

    if(source_code == NULL) {
        error = MJS_FILE_READ_ERROR;
        mjs_prepend_errorf(mjs, error, "failed to read file \"%s\"", path);
        goto clean;
    }

    r = MJS_UNDEFINED;
    error = mjs_exec_internal(mjs, path, source_code, -1, &r);
    free(source_code);

clean:
    if(res != NULL) *res = r;
    return error;
}

mjs_err_t
    mjs_call(struct mjs* mjs, mjs_val_t* res, mjs_val_t func, mjs_val_t this_val, int nargs, ...) {
    va_list ap;
    int i;
    mjs_err_t ret;
    mjs_val_t* args = calloc(nargs, sizeof(mjs_val_t));
    va_start(ap, nargs);
    for(i = 0; i < nargs; i++) {
        args[i] = va_arg(ap, mjs_val_t);
    }
    va_end(ap);

    ret = mjs_apply(mjs, res, func, this_val, nargs, args);

    free(args);
    return ret;
}

mjs_err_t mjs_apply(
    struct mjs* mjs,
    mjs_val_t* res,
    mjs_val_t func,
    mjs_val_t this_val,
    int nargs,
    mjs_val_t* args) {
    mjs_val_t r, prev_this_val, retval_stack_idx, *resp;
    int i;

    if(!mjs_is_function(func) && !mjs_is_foreign(func) && !mjs_is_ffi_sig(func)) {
        return mjs_set_errorf(mjs, MJS_TYPE_ERROR, "calling non-callable");
    }

    LOG(LL_VERBOSE_DEBUG, ("applying func %d", (int)mjs_get_func_addr(func)));

    prev_this_val = mjs->vals.this_obj;

    /* Push callable which will be later replaced with the return value */
    mjs_push(mjs, func);
    resp = vptr(&mjs->stack, -1);

    /* Remember index by which return value should be written */
    retval_stack_idx = mjs_mk_number(mjs, (double)mjs_stack_size(&mjs->stack));

    // Push all arguments
    for(i = 0; i < nargs; i++) {
        mjs_push(mjs, args[i]);
    }

    /* Push this value to arg_stack, call_stack_push_frame() expects that */
    push_mjs_val(&mjs->arg_stack, this_val);

    /* Push call stack frame, just like OP_CALL does that */
    call_stack_push_frame(mjs, MJS_BCODE_OFFSET_EXIT, retval_stack_idx);

    if(mjs_is_foreign(func)) {
        ((void (*)(struct mjs*))mjs_get_ptr(mjs, func))(mjs);
        if(res != NULL) *res = *resp;
    } else if(mjs_is_ffi_sig(func)) {
        mjs_ffi_call2(mjs);
        if(res != NULL) *res = *resp;
    } else {
        size_t addr = mjs_get_func_addr(func);
        mjs_execute(mjs, addr, &r);
        if(res != NULL) *res = r;
    }

    /*
   * If there was an error, we need to restore frame and do the cleanup
   * which is otherwise done by OP_RETURN
   */
    if(mjs->error != MJS_OK) {
        call_stack_restore_frame(mjs);

        // Pop cell at which the returned value should've been written
        mjs_pop(mjs);
    }
    mjs->vals.this_obj = prev_this_val;

    return mjs->error;
}
