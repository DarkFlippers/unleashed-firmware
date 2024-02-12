/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include "common/cs_varint.h"
#include "common/frozen/frozen.h"
#include "mjs_array.h"
#include "mjs_bcode.h"
#include "mjs_core.h"
#include "mjs_internal.h"
#include "mjs_object.h"
#include "mjs_primitive.h"
#include "mjs_string.h"
#include "mjs_util.h"
#include "mjs_tok.h"
#include "mjs_array_buf.h"
#include <furi.h>

const char* mjs_typeof(mjs_val_t v) {
    return mjs_stringify_type(mjs_get_type(v));
}

MJS_PRIVATE const char* mjs_stringify_type(enum mjs_type t) {
    switch(t) {
    case MJS_TYPE_NUMBER:
        return "number";
    case MJS_TYPE_BOOLEAN:
        return "boolean";
    case MJS_TYPE_STRING:
        return "string";
    case MJS_TYPE_OBJECT_ARRAY:
        return "array";
    case MJS_TYPE_OBJECT_GENERIC:
        return "object";
    case MJS_TYPE_FOREIGN:
        return "foreign_ptr";
    case MJS_TYPE_OBJECT_FUNCTION:
        return "function";
    case MJS_TYPE_NULL:
        return "null";
    case MJS_TYPE_UNDEFINED:
        return "undefined";
    case MJS_TYPE_ARRAY_BUF:
        return "array_buf";
    case MJS_TYPE_ARRAY_BUF_VIEW:
        return "data_view";
    default:
        return "???";
    }
}

void mjs_jprintf(mjs_val_t v, struct mjs* mjs, struct json_out* out) {
    if(mjs_is_number(v)) {
        double iv, d = mjs_get_double(mjs, v);
        if(modf(d, &iv) == 0) {
            json_printf(out, "%" INT64_FMT, (int64_t)d);
        } else {
            json_printf(out, "%f", mjs_get_double(mjs, v));
        }
    } else if(mjs_is_boolean(v)) {
        json_printf(out, "%s", mjs_get_bool(mjs, v) ? "true" : "false");
    } else if(mjs_is_string(v)) {
        size_t i, size;
        const char* s = mjs_get_string(mjs, &v, &size);
        for(i = 0; i < size; i++) {
            int ch = ((unsigned char*)s)[i];
            if(isprint(ch)) {
                json_printf(out, "%c", ch);
            } else {
                json_printf(out, "%s%02x", "\\x", ch);
            }
        }
    } else if(mjs_is_array(v)) {
        json_printf(out, "%s", "<array>");
    } else if(mjs_is_object(v)) {
        json_printf(out, "%s", "<object>");
    } else if(mjs_is_foreign(v)) {
        json_printf(
            out, "%s%lx%s", "<foreign_ptr@", (unsigned long)(uintptr_t)mjs_get_ptr(mjs, v), ">");
    } else if(mjs_is_function(v)) {
        json_printf(out, "%s%d%s", "<function@", (int)mjs_get_func_addr(v), ">");
    } else if(mjs_is_null(v)) {
        json_printf(out, "%s", "null");
    } else if(mjs_is_undefined(v)) {
        json_printf(out, "%s", "undefined");
    } else {
        json_printf(out, "%s%" INT64_FMT "%s", "<???", (int64_t)v, ">");
    }
}

void mjs_sprintf(mjs_val_t v, struct mjs* mjs, char* buf, size_t n) {
    struct json_out out = JSON_OUT_BUF(buf, n);
    mjs_jprintf(v, mjs, &out);
}

void mjs_fprintf(mjs_val_t v, struct mjs* mjs, FILE* fp) {
    struct json_out out = JSON_OUT_FILE(fp);
    mjs_jprintf(v, mjs, &out);
}

MJS_PRIVATE const char* opcodetostr(uint8_t opcode) {
    static const char* names[] = {
        "NOP",
        "DROP",
        "DUP",
        "SWAP",
        "JMP",
        "JMP_TRUE",
        "JMP_NEUTRAL_TRUE",
        "JMP_FALSE",
        "JMP_NEUTRAL_FALSE",
        "FIND_SCOPE",
        "PUSH_SCOPE",
        "PUSH_STR",
        "PUSH_TRUE",
        "PUSH_FALSE",
        "PUSH_INT",
        "PUSH_DBL",
        "PUSH_NULL",
        "PUSH_UNDEF",
        "PUSH_OBJ",
        "PUSH_ARRAY",
        "PUSH_FUNC",
        "PUSH_THIS",
        "GET",
        "CREATE",
        "EXPR",
        "APPEND",
        "SET_ARG",
        "NEW_SCOPE",
        "DEL_SCOPE",
        "CALL",
        "RETURN",
        "LOOP",
        "BREAK",
        "CONTINUE",
        "SETRETVAL",
        "EXIT",
        "BCODE_HDR",
        "ARGS",
        "FOR_IN_NEXT",
    };
    const char* name = "???";
    assert(ARRAY_SIZE(names) == OP_MAX);
    if(opcode < ARRAY_SIZE(names)) name = names[opcode];
    return name;
}

MJS_PRIVATE size_t
    mjs_disasm_single(const uint8_t* code, size_t i, MjsPrintCallback print_cb, void* print_ctx) {
    char buf[40];
    size_t start_i = i;
    size_t llen;
    uint64_t n;

    furi_assert(print_cb);

    snprintf(buf, sizeof(buf), "%-3u\t%-8s", (unsigned)i, opcodetostr(code[i]));

    switch(code[i]) {
    case OP_PUSH_FUNC: {
        cs_varint_decode(&code[i + 1], ~0, &n, &llen);
        print_cb(print_ctx, "%s %04u", buf, (unsigned)(i - n));
        i += llen;
        break;
    }
    case OP_PUSH_INT: {
        cs_varint_decode(&code[i + 1], ~0, &n, &llen);
        print_cb(print_ctx, "%s\t%lu", buf, (unsigned long)n);
        i += llen;
        break;
    }
    case OP_SET_ARG: {
        size_t llen2;
        uint64_t arg_no;
        cs_varint_decode(&code[i + 1], ~0, &arg_no, &llen);
        cs_varint_decode(&code[i + llen + 1], ~0, &n, &llen2);
        print_cb(
            print_ctx, "%s\t[%.*s] %u", buf, (int)n, code + i + 1 + llen + llen2, (unsigned)arg_no);
        i += llen + llen2 + n;
        break;
    }
    case OP_PUSH_STR:
    case OP_PUSH_DBL: {
        cs_varint_decode(&code[i + 1], ~0, &n, &llen);
        print_cb(print_ctx, "%s\t[%.*s]", buf, (int)n, code + i + 1 + llen);
        i += llen + n;
        break;
    }
    case OP_JMP:
    case OP_JMP_TRUE:
    case OP_JMP_NEUTRAL_TRUE:
    case OP_JMP_FALSE:
    case OP_JMP_NEUTRAL_FALSE: {
        cs_varint_decode(&code[i + 1], ~0, &n, &llen);
        print_cb(
            print_ctx,
            "%s\t%u",
            buf,
            (unsigned)(i + n + llen + 1 /* becaue i will be incremented on the usual terms */));
        i += llen;
        break;
    }
    case OP_LOOP: {
        size_t l1, l2;
        uint64_t n1, n2;
        cs_varint_decode(&code[i + 1], ~0, &n1, &l1);
        cs_varint_decode(&code[i + l1 + 1], ~0, &n2, &l2);
        print_cb(
            print_ctx,
            "%s\tB:%lu C:%lu (%d)",
            buf,
            (unsigned long)(i + 1 /* OP_LOOP */ + l1 + n1),
            (unsigned long)(i + 1 /* OP_LOOP */ + l1 + l2 + n2),
            (int)i);
        i += l1 + l2;
        break;
    }
    case OP_EXPR: {
        int op = code[i + 1];
        const char* name = "???";
        /* clang-format off */
      switch (op) {
        case TOK_DOT:       name = "."; break;
        case TOK_MINUS:     name = "-"; break;
        case TOK_PLUS:      name = "+"; break;
        case TOK_MUL:       name = "*"; break;
        case TOK_DIV:       name = "/"; break;
        case TOK_REM:       name = "%"; break;
        case TOK_XOR:       name = "^"; break;
        case TOK_AND:       name = "&"; break;
        case TOK_OR:        name = "|"; break;
        case TOK_LSHIFT:    name = "<<"; break;
        case TOK_RSHIFT:    name = ">>"; break;
        case TOK_URSHIFT:   name = ">>>"; break;
        case TOK_UNARY_MINUS:   name = "- (unary)"; break;
        case TOK_UNARY_PLUS:    name = "+ (unary)"; break;
        case TOK_NOT:       name = "!"; break;
        case TOK_TILDA:     name = "~"; break;
        case TOK_EQ:        name = "=="; break;
        case TOK_NE:        name = "!="; break;
        case TOK_EQ_EQ:     name = "==="; break;
        case TOK_NE_NE:     name = "!=="; break;
        case TOK_LT:        name = "<"; break;
        case TOK_GT:        name = ">"; break;
        case TOK_LE:        name = "<="; break;
        case TOK_GE:        name = ">="; break;
        case TOK_ASSIGN:    name = "="; break;
        case TOK_POSTFIX_PLUS:  name = "++ (postfix)"; break;
        case TOK_POSTFIX_MINUS: name = "-- (postfix)"; break;
        case TOK_MINUS_MINUS:   name = "--"; break;
        case TOK_PLUS_PLUS:     name = "++"; break;
        case TOK_LOGICAL_AND:   name = "&&"; break;
        case TOK_LOGICAL_OR:    name = "||"; break;
        case TOK_KEYWORD_TYPEOF:  name = "typeof"; break;
        case TOK_PLUS_ASSIGN:     name = "+="; break;
        case TOK_MINUS_ASSIGN:    name = "-="; break;
        case TOK_MUL_ASSIGN:      name = "*="; break;
        case TOK_DIV_ASSIGN:      name = "/="; break;
        case TOK_REM_ASSIGN:      name = "%="; break;
        case TOK_XOR_ASSIGN:      name = "^="; break;
        case TOK_AND_ASSIGN:      name = "&="; break;
        case TOK_OR_ASSIGN:       name = "|="; break;
        case TOK_LSHIFT_ASSIGN:   name = "<<="; break;
        case TOK_RSHIFT_ASSIGN:   name = ">>="; break;
        case TOK_URSHIFT_ASSIGN:  name = ">>>="; break;
      }
        /* clang-format on */
        print_cb(print_ctx, "%s\t%s", buf, name);
        i++;
        break;
    }
    case OP_BCODE_HEADER: {
        size_t start = 0;
        mjs_header_item_t map_offset = 0, total_size = 0;
        start = i;
        memcpy(&total_size, &code[i + 1], sizeof(total_size));
        memcpy(
            &map_offset,
            &code[i + 1 + MJS_HDR_ITEM_MAP_OFFSET * sizeof(total_size)],
            sizeof(map_offset));
        i += sizeof(mjs_header_item_t) * MJS_HDR_ITEMS_CNT;
        print_cb(
            print_ctx,
            "%s\t[%s] end:%lu map_offset: %lu",
            buf,
            &code[i + 1],
            (unsigned long)start + total_size,
            (unsigned long)start + map_offset);
        i += strlen((char*)(code + i + 1)) + 1;
        break;
    }
    default:
        print_cb(print_ctx, "%s", buf);
        break;
    }
    return i - start_i;
}

void mjs_disasm(const uint8_t* code, size_t len, MjsPrintCallback print_cb, void* print_ctx) {
    size_t i, start = 0;
    mjs_header_item_t map_offset = 0, total_size = 0;

    for(i = 0; i < len; i++) {
        size_t delta = mjs_disasm_single(code, i, print_cb, print_ctx);
        if(code[i] == OP_BCODE_HEADER) {
            start = i;
            memcpy(&total_size, &code[i + 1], sizeof(total_size));
            memcpy(
                &map_offset,
                &code[i + 1 + MJS_HDR_ITEM_MAP_OFFSET * sizeof(total_size)],
                sizeof(map_offset));
        }

        i += delta;

        if(map_offset > 0 && i == start + map_offset) {
            i = start + total_size - 1;
            continue;
        }
    }
}

void mjs_disasm_all(struct mjs* mjs, MjsPrintCallback print_cb, void* print_ctx) {
    int parts_cnt = mjs_bcode_parts_cnt(mjs);
    for(int i = 0; i < parts_cnt; i++) {
        struct mjs_bcode_part* bp = mjs_bcode_part_get(mjs, i);
        mjs_disasm((uint8_t*)bp->data.p, bp->data.len, print_cb, print_ctx);
    }
}

static void mjs_dump_obj_stack(
    struct mjs* mjs,
    const char* name,
    const struct mbuf* m,
    MjsPrintCallback print_cb,
    void* print_ctx) {
    char buf[50];
    size_t i, n;
    n = mjs_stack_size(m);
    print_cb(print_ctx, "%12s (%d elems): ", name, (int)n);
    for(i = 0; i < n; i++) {
        mjs_sprintf(((mjs_val_t*)m->buf)[i], mjs, buf, sizeof(buf));
        print_cb(print_ctx, "%34s", buf);
    }
}

void mjs_dump(struct mjs* mjs, int do_disasm, MjsPrintCallback print_cb, void* print_ctx) {
    print_cb(print_ctx, "------- MJS VM DUMP BEGIN");
    mjs_dump_obj_stack(mjs, "DATA_STACK", &mjs->stack, print_cb, print_ctx);
    mjs_dump_obj_stack(mjs, "CALL_STACK", &mjs->call_stack, print_cb, print_ctx);
    mjs_dump_obj_stack(mjs, "SCOPES", &mjs->scopes, print_cb, print_ctx);
    mjs_dump_obj_stack(mjs, "LOOP_OFFSETS", &mjs->loop_addresses, print_cb, print_ctx);
    mjs_dump_obj_stack(mjs, "ARG_STACK", &mjs->arg_stack, print_cb, print_ctx);
    if(do_disasm) {
        int parts_cnt = mjs_bcode_parts_cnt(mjs);
        int i;
        print_cb(print_ctx, "%23s", "CODE:");
        for(i = 0; i < parts_cnt; i++) {
            struct mjs_bcode_part* bp = mjs_bcode_part_get(mjs, i);
            mjs_disasm((uint8_t*)bp->data.p, bp->data.len, print_cb, print_ctx);
        }
    }
    print_cb(print_ctx, "------- MJS VM DUMP END");
}

MJS_PRIVATE int mjs_check_arg(
    struct mjs* mjs,
    int arg_num,
    const char* arg_name,
    enum mjs_type expected_type,
    mjs_val_t* parg) {
    mjs_val_t arg = MJS_UNDEFINED;
    enum mjs_type actual_type;

    if(arg_num >= 0) {
        int nargs = mjs_nargs(mjs);
        if(nargs < arg_num + 1) {
            mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "missing argument %s", arg_name);
            return 0;
        }

        arg = mjs_arg(mjs, arg_num);
    } else {
        /* use `this` */
        arg = mjs->vals.this_obj;
    }

    actual_type = mjs_get_type(arg);
    if(actual_type != expected_type) {
        mjs_prepend_errorf(
            mjs,
            MJS_TYPE_ERROR,
            "%s should be a %s, %s given",
            arg_name,
            mjs_stringify_type(expected_type),
            mjs_stringify_type(actual_type));
        return 0;
    }

    if(parg != NULL) {
        *parg = arg;
    }

    return 1;
}

MJS_PRIVATE int mjs_normalize_idx(int idx, int size) {
    if(idx < 0) {
        idx = size + idx;
        if(idx < 0) {
            idx = 0;
        }
    }
    if(idx > size) {
        idx = size;
    }
    return idx;
}

MJS_PRIVATE const char* mjs_get_bcode_filename(struct mjs* mjs, struct mjs_bcode_part* bp) {
    (void)mjs;
    return bp->data.p + 1 /* OP_BCODE_HEADER */ + sizeof(mjs_header_item_t) * MJS_HDR_ITEMS_CNT;
}

const char* mjs_get_bcode_filename_by_offset(struct mjs* mjs, int offset) {
    const char* ret = NULL;
    struct mjs_bcode_part* bp = mjs_bcode_part_get_by_offset(mjs, offset);
    if(bp != NULL) {
        ret = mjs_get_bcode_filename(mjs, bp);
    }
    return ret;
}

int mjs_get_lineno_by_offset(struct mjs* mjs, int offset) {
    size_t llen;
    uint64_t map_len;
    int prev_line_no, ret = 1;
    struct mjs_bcode_part* bp = mjs_bcode_part_get_by_offset(mjs, offset);
    uint8_t *p, *pe;
    if(bp != NULL) {
        mjs_header_item_t map_offset, bcode_offset;
        memcpy(
            &map_offset,
            bp->data.p + 1 /* OP_BCODE_HEADER */ +
                sizeof(mjs_header_item_t) * MJS_HDR_ITEM_MAP_OFFSET,
            sizeof(map_offset));

        memcpy(
            &bcode_offset,
            bp->data.p + 1 /* OP_BCODE_HEADER */ +
                sizeof(mjs_header_item_t) * MJS_HDR_ITEM_BCODE_OFFSET,
            sizeof(bcode_offset));

        offset -= (1 /* OP_BCODE_HEADER */ + bcode_offset) + bp->start_idx;

        /* get pointer to the length of the map followed by the map itself */
        p = (uint8_t*)bp->data.p + 1 /* OP_BCODE_HEADER */ + map_offset;

        cs_varint_decode(p, ~0, &map_len, &llen);
        p += llen;
        pe = p + map_len;

        prev_line_no = 1;
        while(p < pe) {
            uint64_t cur_offset, line_no;
            cs_varint_decode(p, ~0, &cur_offset, &llen);
            p += llen;
            cs_varint_decode(p, ~0, &line_no, &llen);
            p += llen;

            if(cur_offset >= (uint64_t)offset) {
                ret = prev_line_no;
                break;
            }
            prev_line_no = line_no;
        }
    }
    return ret;
}

int mjs_get_offset_by_call_frame_num(struct mjs* mjs, int cf_num) {
    int ret = -1;
    if(cf_num == 0) {
        /* Return current bcode offset */
        ret = mjs->cur_bcode_offset;
    } else if(
        cf_num > 0 &&
        mjs->call_stack.len >= sizeof(mjs_val_t) * CALL_STACK_FRAME_ITEMS_CNT * cf_num) {
        /* Get offset from the call_stack */
        int pos = CALL_STACK_FRAME_ITEM_RETURN_ADDR + CALL_STACK_FRAME_ITEMS_CNT * (cf_num - 1);
        mjs_val_t val = *vptr(&mjs->call_stack, -1 - pos);
        ret = mjs_get_int(mjs, val);
    }
    return ret;
}

mjs_err_t mjs_to_string(struct mjs* mjs, mjs_val_t* v, char** p, size_t* sizep, int* need_free) {
    mjs_err_t ret = MJS_OK;

    *p = NULL;
    *sizep = 0;
    *need_free = 0;

    if(mjs_is_string(*v)) {
        *p = (char*)mjs_get_string(mjs, v, sizep);
    } else if(mjs_is_number(*v)) {
        char buf[50] = "";
        struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));
        mjs_jprintf(*v, mjs, &out);
        *sizep = strlen(buf);
        *p = malloc(*sizep + 1);
        if(*p == NULL) {
            ret = MJS_OUT_OF_MEMORY;
            goto clean;
        }
        memmove(*p, buf, *sizep + 1);
        *need_free = 1;
    } else if(mjs_is_boolean(*v)) {
        if(mjs_get_bool(mjs, *v)) {
            *p = "true";
            *sizep = 4;
        } else {
            *p = "false";
            *sizep = 5;
        }
    } else if(mjs_is_undefined(*v)) {
        *p = "undefined";
        *sizep = 9;
    } else if(mjs_is_null(*v)) {
        *p = "null";
        *sizep = 4;
    } else if(mjs_is_object(*v)) {
        ret = MJS_TYPE_ERROR;
        mjs_set_errorf(mjs, ret, "conversion from object to string is not supported");
    } else if(mjs_is_foreign(*v)) {
        *p = "TODO_foreign";
        *sizep = 12;
    } else if(mjs_is_typed_array(*v)) {
        *p = "TODO_typed_array";
        *sizep = 16;
    } else {
        ret = MJS_TYPE_ERROR;
        mjs_set_errorf(mjs, ret, "unknown type to convert to string");
    }

clean:
    return ret;
}

mjs_val_t mjs_to_boolean_v(struct mjs* mjs, mjs_val_t v) {
    size_t len;
    int is_truthy;

    is_truthy = ((mjs_is_boolean(v) && mjs_get_bool(mjs, v)) ||
                 (mjs_is_number(v) && mjs_get_double(mjs, v) != (double)0.0) ||
                 (mjs_is_string(v) && mjs_get_string(mjs, &v, &len) && len > 0) ||
                 (mjs_is_function(v)) || (mjs_is_foreign(v)) || (mjs_is_object(v))) &&
                v != MJS_TAG_NAN;

    return mjs_mk_boolean(mjs, is_truthy);
}

int mjs_is_truthy(struct mjs* mjs, mjs_val_t v) {
    return mjs_get_bool(mjs, mjs_to_boolean_v(mjs, v));
}
