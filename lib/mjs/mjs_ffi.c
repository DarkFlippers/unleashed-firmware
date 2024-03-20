/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include "common/mg_str.h"

#include "ffi/ffi.h"
#include "mjs_core.h"
#include "mjs_exec.h"
#include "mjs_internal.h"
#include "mjs_primitive.h"
#include "mjs_string.h"
#include "mjs_util.h"

/*
 * on linux this is enabled only if __USE_GNU is defined, but we cannot set it
 * because dlfcn could have been included already.
 */
#ifndef RTLD_DEFAULT
#define RTLD_DEFAULT NULL
#endif

static ffi_fn_t* get_cb_impl_by_signature(const mjs_ffi_sig_t* sig);

/*
 * Data of the two related arguments: callback function pointer and the
 * userdata for it
 */
struct cbdata {
    /* JS callback function */
    mjs_val_t func;
    /* JS userdata */
    mjs_val_t userdata;

    /* index of the function pointer param */
    int8_t func_idx;
    /* index of the userdata param */
    int8_t userdata_idx;
};

void mjs_set_ffi_resolver(struct mjs* mjs, mjs_ffi_resolver_t* dlsym, void* handle) {
    mjs->dlsym = dlsym;
    mjs->dlsym_handle = handle;
}

void* mjs_ffi_resolve(struct mjs* mjs, const char* symbol) {
    if(mjs->dlsym) {
        return mjs->dlsym(mjs->dlsym_handle, symbol);
    }
    return NULL;
}

static mjs_ffi_ctype_t parse_cval_type(struct mjs* mjs, const char* s, const char* e) {
    struct mg_str ms = MG_NULL_STR;
    /* Trim leading and trailing whitespace */
    while(s < e && isspace((int)*s)) s++;
    while(e > s && isspace((int)e[-1])) e--;
    ms.p = s;
    ms.len = e - s;
    if(mg_vcmp(&ms, "void") == 0) {
        return MJS_FFI_CTYPE_NONE;
    } else if(mg_vcmp(&ms, "userdata") == 0) {
        return MJS_FFI_CTYPE_USERDATA;
    } else if(mg_vcmp(&ms, "int") == 0) {
        return MJS_FFI_CTYPE_INT;
    } else if(mg_vcmp(&ms, "bool") == 0) {
        return MJS_FFI_CTYPE_BOOL;
    } else if(mg_vcmp(&ms, "double") == 0) {
        return MJS_FFI_CTYPE_DOUBLE;
    } else if(mg_vcmp(&ms, "float") == 0) {
        return MJS_FFI_CTYPE_FLOAT;
    } else if(mg_vcmp(&ms, "char*") == 0 || mg_vcmp(&ms, "char *") == 0) {
        return MJS_FFI_CTYPE_CHAR_PTR;
    } else if(mg_vcmp(&ms, "void*") == 0 || mg_vcmp(&ms, "void *") == 0) {
        return MJS_FFI_CTYPE_VOID_PTR;
    } else if(mg_vcmp(&ms, "struct mg_str") == 0) {
        return MJS_FFI_CTYPE_STRUCT_MG_STR;
    } else if(mg_vcmp(&ms, "struct mg_str *") == 0 || mg_vcmp(&ms, "struct mg_str*") == 0) {
        return MJS_FFI_CTYPE_STRUCT_MG_STR_PTR;
    } else {
        mjs_prepend_errorf(
            mjs, MJS_TYPE_ERROR, "failed to parse val type \"%.*s\"", (int)ms.len, ms.p);
        return MJS_FFI_CTYPE_INVALID;
    }
}

static const char* find_paren(const char* s, const char* e) {
    for(; s < e; s++) {
        if(*s == '(') return s;
    }
    return NULL;
}

static const char* find_closing_paren(const char* s, const char* e) {
    int nesting = 1;
    while(s < e) {
        if(*s == '(') {
            nesting++;
        } else if(*s == ')') {
            if(--nesting == 0) break;
        }
        s++;
    }
    return (s < e ? s : NULL);
}

MJS_PRIVATE mjs_err_t mjs_parse_ffi_signature(
    struct mjs* mjs,
    const char* s,
    int sig_len,
    mjs_ffi_sig_t* sig,
    enum ffi_sig_type sig_type) {
    mjs_err_t ret = MJS_OK;
    int vtidx = 0;
    const char *cur, *e, *tmp_e, *tmp;
    struct mg_str rt = MG_NULL_STR, fn = MG_NULL_STR, args = MG_NULL_STR;
    mjs_ffi_ctype_t val_type = MJS_FFI_CTYPE_INVALID;
    if(sig_len == ~0) {
        sig_len = strlen(s);
    }
    e = s + sig_len;

    mjs_ffi_sig_init(sig);

    /* Skip leading spaces */
    for(cur = s; cur < e && isspace((int)*cur); cur++)
        ;

    /* FInd the first set of parens */
    tmp_e = find_paren(cur, e);
    if(tmp_e == NULL || tmp_e - s < 2) {
        ret = MJS_TYPE_ERROR;
        mjs_prepend_errorf(mjs, ret, "1");
        goto clean;
    }
    tmp = find_closing_paren(tmp_e + 1, e);
    if(tmp == NULL) {
        ret = MJS_TYPE_ERROR;
        mjs_prepend_errorf(mjs, ret, "2");
        goto clean;
    }

    /* Now see if we have a second set of parens */
    args.p = find_paren(tmp + 1, e);
    if(args.p == NULL) {
        /* We don't - it's a regular function signature */
        fn.p = tmp_e - 1;
        while(fn.p > cur && isspace((int)*fn.p)) fn.p--;
        while(fn.p > cur && (isalnum((int)*fn.p) || *fn.p == '_')) {
            fn.p--;
            fn.len++;
        }
        fn.p++;
        rt.p = cur;
        rt.len = fn.p - rt.p;
        /* Stuff inside parens is args */
        args.p = tmp_e + 1;
        args.len = tmp - args.p;
    } else {
        /* We do - it's a function pointer, like void (*foo)(...).
     * Stuff inside the first pair of parens is the function name */
        fn.p = tmp + 1;
        fn.len = args.p - tmp;
        rt.p = cur;
        rt.len = tmp_e - rt.p;
        args.p++;
        tmp = find_closing_paren(args.p, e);
        if(tmp == NULL) {
            ret = MJS_TYPE_ERROR;
            mjs_prepend_errorf(mjs, ret, "3");
            goto clean;
        }
        args.len = tmp - args.p;
        /*
     * We ignore the name and leave sig->fn NULL here, but it will later be
     * set to the appropriate callback implementation.
     */
        sig->is_callback = 1;
    }

    val_type = parse_cval_type(mjs, rt.p, rt.p + rt.len);
    if(val_type == MJS_FFI_CTYPE_INVALID) {
        ret = mjs->error;
        goto clean;
    }
    mjs_ffi_sig_set_val_type(sig, vtidx++, val_type);

    /* Parse function name {{{ */
    if(!sig->is_callback) {
        char buf[100];
        if(mjs->dlsym == NULL) {
            ret = MJS_TYPE_ERROR;
            mjs_prepend_errorf(mjs, ret, "resolver is not set, call mjs_set_ffi_resolver");
            goto clean;
        }

        snprintf(buf, sizeof(buf), "%.*s", (int)fn.len, fn.p);
        sig->fn = (ffi_fn_t*)mjs->dlsym(mjs->dlsym_handle, buf);
        if(sig->fn == NULL) {
            ret = MJS_TYPE_ERROR;
            mjs_prepend_errorf(mjs, ret, "dlsym('%s') failed", buf);
            goto clean;
        }
    } else {
        tmp_e = strchr(tmp_e, ')');
        if(tmp_e == NULL) {
            ret = MJS_TYPE_ERROR;
            goto clean;
        }
    }

    /* Advance cur to the beginning of the arg list */
    cur = tmp_e = args.p;

    /* Parse all args {{{ */
    while(tmp_e - args.p < (ptrdiff_t)args.len) {
        int level = 0; /* nested parens level */
        int is_fp = 0; /* set to 1 is current arg is a callback function ptr */
        tmp_e = cur;

        /* Advance tmp_e until the next arg separator */
        while(*tmp_e && (level > 0 || (*tmp_e != ',' && *tmp_e != ')'))) {
            switch(*tmp_e) {
            case '(':
                level++;
                /*
           * only function pointer params can have parens, so, set the flag
           * that it's going to be a function pointer
           */
                is_fp = 1;
                break;
            case ')':
                level--;
                break;
            }
            tmp_e++;
        }

        if(tmp_e == cur) break;

        /* Parse current arg */
        if(is_fp) {
            /* Current argument is a callback function pointer */
            if(sig->cb_sig != NULL) {
                /*
         * We already have parsed some callback argument. Currently we don't
         * support more than one callback argument, so, return error
         * TODO(dfrank): probably improve
         */
                ret = MJS_TYPE_ERROR;
                mjs_prepend_errorf(mjs, ret, "only one callback is allowed");
                goto clean;
            }

            sig->cb_sig = calloc(sizeof(*sig->cb_sig), 1);
            ret = mjs_parse_ffi_signature(mjs, cur, tmp_e - cur, sig->cb_sig, FFI_SIG_CALLBACK);
            if(ret != MJS_OK) {
                mjs_ffi_sig_free(sig->cb_sig);
                free(sig->cb_sig);
                sig->cb_sig = NULL;
                goto clean;
            }
            val_type = MJS_FFI_CTYPE_CALLBACK;
        } else {
            /* Some non-function argument */
            val_type = parse_cval_type(mjs, cur, tmp_e);
            if(val_type == MJS_FFI_CTYPE_INVALID) {
                /* parse_cval_type() has already set error message */
                ret = MJS_TYPE_ERROR;
                goto clean;
            }
        }

        if(!mjs_ffi_sig_set_val_type(sig, vtidx++, val_type)) {
            ret = MJS_TYPE_ERROR;
            mjs_prepend_errorf(mjs, ret, "too many callback args");
            goto clean;
        }

        if(*tmp_e == ',') {
            /* Advance cur to the next argument */
            cur = tmp_e + 1;
            while(*cur == ' ') cur++;
        } else {
            /* No more arguments */
            break;
        }
    }
    /* }}} */

    /* Analyze the results and see if they are obviously wrong */
    mjs_ffi_sig_validate(mjs, sig, sig_type);
    if(!sig->is_valid) {
        ret = MJS_TYPE_ERROR;
        goto clean;
    }

    /* If the signature represents a callback, find the suitable implementation */
    if(sig->is_callback) {
        sig->fn = get_cb_impl_by_signature(sig);
        if(sig->fn == NULL) {
            ret = MJS_TYPE_ERROR;
            mjs_prepend_errorf(
                mjs,
                ret,
                "the callback signature is valid, but there's "
                "no existing callback implementation for it");
            goto clean;
        }
    }

clean:
    if(ret != MJS_OK) {
        mjs_prepend_errorf(mjs, ret, "bad ffi signature: \"%.*s\"", sig_len, s);
        sig->is_valid = 0;
    }
    return ret;
}

/* C callbacks implementation {{{ */

/* An argument or a return value for C callback impl */
union ffi_cb_data_val {
    void* p;
    uintptr_t w;
    double d;
    float f;
};

struct ffi_cb_data {
    union ffi_cb_data_val args[MJS_CB_ARGS_MAX_CNT];
};

static union ffi_cb_data_val ffi_cb_impl_generic(void* param, struct ffi_cb_data* data) {
    struct mjs_ffi_cb_args* cbargs = (struct mjs_ffi_cb_args*)param;
    mjs_val_t *args, res = MJS_UNDEFINED;
    union ffi_cb_data_val ret;
    int i;
    struct mjs* mjs = cbargs->mjs;
    mjs_ffi_ctype_t return_ctype = MJS_FFI_CTYPE_NONE;
    mjs_err_t err;

    memset(&ret, 0, sizeof(ret));
    mjs_own(mjs, &res);

    /* There must be at least one argument: a userdata */
    assert(cbargs->sig.args_cnt > 0);

    /* Create JS arguments */
    args = calloc(1, sizeof(mjs_val_t) * cbargs->sig.args_cnt);
    for(i = 0; i < cbargs->sig.args_cnt; i++) {
        mjs_ffi_ctype_t val_type =
            cbargs->sig.val_types[i + 1 /* first val_type is return value type */];
        switch(val_type) {
        case MJS_FFI_CTYPE_USERDATA:
            args[i] = cbargs->userdata;
            break;
        case MJS_FFI_CTYPE_INT:
            args[i] = mjs_mk_number(mjs, (double)data->args[i].w);
            break;
        case MJS_FFI_CTYPE_BOOL:
            args[i] = mjs_mk_boolean(mjs, !!data->args[i].w);
            break;
        case MJS_FFI_CTYPE_CHAR_PTR: {
            const char* s = (char*)data->args[i].w;
            if(s == NULL) s = "";
            args[i] = mjs_mk_string(mjs, s, ~0, 1);
            break;
        }
        case MJS_FFI_CTYPE_VOID_PTR:
            args[i] = mjs_mk_foreign(mjs, (void*)data->args[i].w);
            break;
        case MJS_FFI_CTYPE_DOUBLE:
            args[i] = mjs_mk_number(mjs, data->args[i].d);
            break;
        case MJS_FFI_CTYPE_FLOAT:
            args[i] = mjs_mk_number(mjs, data->args[i].f);
            break;
        case MJS_FFI_CTYPE_STRUCT_MG_STR_PTR: {
            struct mg_str* s = (struct mg_str*)(void*)data->args[i].w;
            args[i] = mjs_mk_string(mjs, s->p, s->len, 1);
            break;
        }
        default:
            /* should never be here */
            LOG(LL_ERROR, ("unexpected val type for arg #%d: %d\n", i, val_type));
            abort();
        }
    }

    /*
   * save return ctype outside of `cbargs` before calling the callback, because
   * callback might call `ffi_cb_free()`, which will effectively invalidate
   * `cbargs`
   */
    return_ctype = cbargs->sig.val_types[0];

    /* Call JS function */
    LOG(LL_VERBOSE_DEBUG,
        ("calling JS callback void-void %d from C", mjs_get_int(mjs, cbargs->func)));
    err = mjs_apply(mjs, &res, cbargs->func, MJS_UNDEFINED, cbargs->sig.args_cnt, args);
    /*
   * cbargs might be invalidated by the callback (if it called ffi_cb_free), so
   * null it out
   */
    cbargs = NULL;
    if(err != MJS_OK) {
        /*
     * There's not much we can do about the error here; let's at least print it
     */
        mjs_print_error(mjs, stderr, "MJS callback error", 1 /* print_stack_trace */);

        goto clean;
    }

    /* Get return value, if needed */
    switch(return_ctype) {
    case MJS_FFI_CTYPE_NONE:
        /* do nothing */
        break;
    case MJS_FFI_CTYPE_INT:
        ret.w = mjs_get_int(mjs, res);
        break;
    case MJS_FFI_CTYPE_BOOL:
        ret.w = mjs_get_bool(mjs, res);
        break;
    case MJS_FFI_CTYPE_VOID_PTR:
        ret.p = mjs_get_ptr(mjs, res);
        break;
    case MJS_FFI_CTYPE_DOUBLE:
        ret.d = mjs_get_double(mjs, res);
        break;
    case MJS_FFI_CTYPE_FLOAT:
        ret.f = (float)mjs_get_double(mjs, res);
        break;
    default:
        /* should never be here */
        LOG(LL_ERROR, ("unexpected return val type %d\n", return_ctype));
        abort();
    }

clean:
    free(args);
    mjs_disown(mjs, &res);
    return ret;
}

static void ffi_init_cb_data_wwww(
    struct ffi_cb_data* data,
    uintptr_t w0,
    uintptr_t w1,
    uintptr_t w2,
    uintptr_t w3,
    uintptr_t w4,
    uintptr_t w5) {
    memset(data, 0, sizeof(*data));
    data->args[0].w = w0;
    data->args[1].w = w1;
    data->args[2].w = w2;
    data->args[3].w = w3;
    data->args[4].w = w4;
    data->args[5].w = w5;
}

static uintptr_t ffi_cb_impl_wpwwwww(
    uintptr_t w0,
    uintptr_t w1,
    uintptr_t w2,
    uintptr_t w3,
    uintptr_t w4,
    uintptr_t w5) {
    struct ffi_cb_data data;
    ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
    return ffi_cb_impl_generic((void*)w0, &data).w;
}

static uintptr_t ffi_cb_impl_wwpwwww(
    uintptr_t w0,
    uintptr_t w1,
    uintptr_t w2,
    uintptr_t w3,
    uintptr_t w4,
    uintptr_t w5) {
    struct ffi_cb_data data;
    ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
    return ffi_cb_impl_generic((void*)w1, &data).w;
}

static uintptr_t ffi_cb_impl_wwwpwww(
    uintptr_t w0,
    uintptr_t w1,
    uintptr_t w2,
    uintptr_t w3,
    uintptr_t w4,
    uintptr_t w5) {
    struct ffi_cb_data data;
    ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
    return ffi_cb_impl_generic((void*)w2, &data).w;
}

static uintptr_t ffi_cb_impl_wwwwpww(
    uintptr_t w0,
    uintptr_t w1,
    uintptr_t w2,
    uintptr_t w3,
    uintptr_t w4,
    uintptr_t w5) {
    struct ffi_cb_data data;
    ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
    return ffi_cb_impl_generic((void*)w3, &data).w;
}

static uintptr_t ffi_cb_impl_wwwwwpw(
    uintptr_t w0,
    uintptr_t w1,
    uintptr_t w2,
    uintptr_t w3,
    uintptr_t w4,
    uintptr_t w5) {
    struct ffi_cb_data data;
    ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
    return ffi_cb_impl_generic((void*)w4, &data).w;
}

static uintptr_t ffi_cb_impl_wwwwwwp(
    uintptr_t w0,
    uintptr_t w1,
    uintptr_t w2,
    uintptr_t w3,
    uintptr_t w4,
    uintptr_t w5) {
    struct ffi_cb_data data;
    ffi_init_cb_data_wwww(&data, w0, w1, w2, w3, w4, w5);
    return ffi_cb_impl_generic((void*)w5, &data).w;
}

static uintptr_t ffi_cb_impl_wpd(uintptr_t w0, double d1) {
    struct ffi_cb_data data;

    memset(&data, 0, sizeof(data));
    data.args[0].w = w0;
    data.args[1].d = d1;

    return ffi_cb_impl_generic((void*)w0, &data).w;
}

static uintptr_t ffi_cb_impl_wdp(double d0, uintptr_t w1) {
    struct ffi_cb_data data;

    memset(&data, 0, sizeof(data));
    data.args[0].d = d0;
    data.args[1].w = w1;

    return ffi_cb_impl_generic((void*)w1, &data).w;
}
/* }}} */

static struct mjs_ffi_cb_args**
    ffi_get_matching(struct mjs_ffi_cb_args** plist, mjs_val_t func, mjs_val_t userdata) {
    for(; *plist != NULL; plist = &((*plist)->next)) {
        if((*plist)->func == func && (*plist)->userdata == userdata) {
            break;
        }
    }
    return plist;
}

static ffi_fn_t* get_cb_impl_by_signature(const mjs_ffi_sig_t* sig) {
    if(sig->is_valid) {
        int i;
        int double_cnt = 0;
        int float_cnt = 0;
        int userdata_idx = 0 /* not a valid value: index 0 means return value */;

        for(i = 1 /*0th item is a return value*/; i < MJS_CB_SIGNATURE_MAX_SIZE; i++) {
            mjs_ffi_ctype_t type = sig->val_types[i];
            switch(type) {
            case MJS_FFI_CTYPE_DOUBLE:
                double_cnt++;
                break;
            case MJS_FFI_CTYPE_FLOAT:
                float_cnt++;
                break;
            case MJS_FFI_CTYPE_USERDATA:
                assert(userdata_idx == 0); /* Otherwise is_valid should be 0 */
                userdata_idx = i;
                break;
            default:
                break;
            }
        }

        if(float_cnt > 0) {
            /* TODO(dfrank): add support for floats in callbacks */
            return NULL;
        }

        assert(userdata_idx > 0); /* Otherwise is_valid should be 0 */

        if(sig->args_cnt <= MJS_CB_ARGS_MAX_CNT) {
            if(mjs_ffi_is_regular_word_or_void(sig->val_types[0])) {
                /* Return type is a word or void */
                switch(double_cnt) {
                case 0:
                    /* No double arguments */
                    switch(userdata_idx) {
                    case 1:
                        return (ffi_fn_t*)ffi_cb_impl_wpwwwww;
                    case 2:
                        return (ffi_fn_t*)ffi_cb_impl_wwpwwww;
                    case 3:
                        return (ffi_fn_t*)ffi_cb_impl_wwwpwww;
                    case 4:
                        return (ffi_fn_t*)ffi_cb_impl_wwwwpww;
                    case 5:
                        return (ffi_fn_t*)ffi_cb_impl_wwwwwpw;
                    case 6:
                        return (ffi_fn_t*)ffi_cb_impl_wwwwwwp;
                    default:
                        /* should never be here */
                        abort();
                    }
                    break;
                case 1:
                    /* 1 double argument */
                    switch(userdata_idx) {
                    case 1:
                        return (ffi_fn_t*)ffi_cb_impl_wpd;
                    case 2:
                        return (ffi_fn_t*)ffi_cb_impl_wdp;
                    }
                    break;
                }
            }
        } else {
            /* Too many arguments for the built-in callback impls */
            /* TODO(dfrank): add support for custom app-dependent resolver */
        }
    }

    return NULL;
}

MJS_PRIVATE mjs_val_t mjs_ffi_sig_to_value(struct mjs_ffi_sig* psig) {
    if(psig == NULL) {
        return MJS_NULL;
    } else {
        return mjs_legit_pointer_to_value(psig) | MJS_TAG_FUNCTION_FFI;
    }
}

MJS_PRIVATE int mjs_is_ffi_sig(mjs_val_t v) {
    return (v & MJS_TAG_MASK) == MJS_TAG_FUNCTION_FFI;
}

MJS_PRIVATE struct mjs_ffi_sig* mjs_get_ffi_sig_struct(mjs_val_t v) {
    struct mjs_ffi_sig* ret = NULL;
    assert(mjs_is_ffi_sig(v));
    ret = (struct mjs_ffi_sig*)get_ptr(v);
    return ret;
}

MJS_PRIVATE mjs_val_t mjs_mk_ffi_sig(struct mjs* mjs) {
    struct mjs_ffi_sig* psig = new_ffi_sig(mjs);
    mjs_ffi_sig_init(psig);
    return mjs_ffi_sig_to_value(psig);
}

MJS_PRIVATE void mjs_ffi_sig_destructor(struct mjs* mjs, void* psig) {
    mjs_ffi_sig_free((mjs_ffi_sig_t*)psig);
    (void)mjs;
}

MJS_PRIVATE mjs_err_t mjs_ffi_call(struct mjs* mjs) {
    mjs_err_t e = MJS_OK;
    const char* sig_str = NULL;
    mjs_val_t sig_str_v = mjs_arg(mjs, 0);
    mjs_val_t ret_v = MJS_UNDEFINED;
    struct mjs_ffi_sig* psig = mjs_get_ffi_sig_struct(mjs_mk_ffi_sig(mjs));
    size_t sig_str_len;

    sig_str = mjs_get_string(mjs, &sig_str_v, &sig_str_len);
    e = mjs_parse_ffi_signature(mjs, sig_str, sig_str_len, psig, FFI_SIG_FUNC);
    if(e != MJS_OK) goto clean;
    ret_v = mjs_ffi_sig_to_value(psig);

clean:
    mjs_return(mjs, ret_v);
    return e;
}

MJS_PRIVATE mjs_err_t mjs_ffi_call2(struct mjs* mjs) {
    mjs_err_t ret = MJS_OK;
    mjs_ffi_sig_t* psig = NULL;
    mjs_ffi_ctype_t rtype;
    mjs_val_t sig_v = *vptr(&mjs->stack, mjs_getretvalpos(mjs));

    int i, nargs;
    struct ffi_arg res;
    struct ffi_arg args[FFI_MAX_ARGS_CNT];
    struct cbdata cbdata;

    /* TODO(dfrank): support multiple callbacks */
    mjs_val_t resv = mjs_mk_undefined();

    /*
   * String arguments, needed to support short strings which are packed into
   * mjs_val_t itself
   */
    mjs_val_t argvs[FFI_MAX_ARGS_CNT];
    struct mg_str argvmgstr[FFI_MAX_ARGS_CNT];

    if(mjs_is_ffi_sig(sig_v)) {
        psig = mjs_get_ffi_sig_struct(sig_v);
    } else {
        ret = MJS_TYPE_ERROR;
        mjs_prepend_errorf(mjs, ret, "non-ffi-callable value");
        goto clean;
    }

    memset(&cbdata, 0, sizeof(cbdata));
    cbdata.func_idx = -1;
    cbdata.userdata_idx = -1;

    rtype = psig->val_types[0];

    switch(rtype) {
    case MJS_FFI_CTYPE_DOUBLE:
        res.ctype = FFI_CTYPE_DOUBLE;
        break;
    case MJS_FFI_CTYPE_FLOAT:
        res.ctype = FFI_CTYPE_FLOAT;
        break;
    case MJS_FFI_CTYPE_BOOL:
        res.ctype = FFI_CTYPE_BOOL;
        break;
    case MJS_FFI_CTYPE_USERDATA:
    case MJS_FFI_CTYPE_INT:
    case MJS_FFI_CTYPE_CHAR_PTR:
    case MJS_FFI_CTYPE_VOID_PTR:
    case MJS_FFI_CTYPE_NONE:
        res.ctype = FFI_CTYPE_WORD;
        break;

    case MJS_FFI_CTYPE_INVALID:
        ret = MJS_TYPE_ERROR;
        mjs_prepend_errorf(mjs, ret, "wrong ffi return type");
        goto clean;
    }
    res.v.i = 0;

    nargs = mjs_stack_size(&mjs->stack) - mjs_get_int(mjs, vtop(&mjs->call_stack));

    if(nargs != psig->args_cnt) {
        ret = MJS_TYPE_ERROR;
        mjs_prepend_errorf(
            mjs, ret, "got %d actuals, but function takes %d args", nargs, psig->args_cnt);
        goto clean;
    }

    for(i = 0; i < nargs; i++) {
        mjs_val_t arg = mjs_arg(mjs, i);

        switch(psig->val_types[1 /* retval type */ + i]) {
        case MJS_FFI_CTYPE_NONE:
            /*
         * Void argument: in any case, it's an error, because if C function
         * takes no arguments, then the FFI-ed JS function should be called
         * without any arguments, and thus we'll not face "void" here.
         */
            ret = MJS_TYPE_ERROR;
            if(i == 0) {
                /* FFI signature is correct, but invocation is wrong */
                mjs_prepend_errorf(mjs, ret, "ffi-ed function takes no arguments");
            } else {
                /*
           * FFI signature is wrong: we can't have "void" as a non-first
           * "argument"
           */
                mjs_prepend_errorf(mjs, ret, "bad ffi arg #%d type: \"void\"", i);
            }

            goto clean;
        case MJS_FFI_CTYPE_USERDATA:
            /* Userdata for the callback */
            if(cbdata.userdata_idx != -1) {
                ret = MJS_TYPE_ERROR;
                mjs_prepend_errorf(
                    mjs, ret, "two or more userdata args: #%d and %d", cbdata.userdata_idx, i);

                goto clean;
            }
            cbdata.userdata = arg;
            cbdata.userdata_idx = i;
            break;
        case MJS_FFI_CTYPE_INT: {
            int intval = 0;
            if(mjs_is_number(arg)) {
                intval = mjs_get_int(mjs, arg);
            } else if(mjs_is_boolean(arg)) {
                intval = mjs_get_bool(mjs, arg);
            } else {
                ret = MJS_TYPE_ERROR;
                mjs_prepend_errorf(
                    mjs,
                    ret,
                    "actual arg #%d is not an int (the type idx is: %s)",
                    i,
                    mjs_typeof(arg));
            }
            ffi_set_word(&args[i], intval);
        } break;
        case MJS_FFI_CTYPE_STRUCT_MG_STR_PTR: {
            if(!mjs_is_string(arg)) {
                ret = MJS_TYPE_ERROR;
                mjs_prepend_errorf(
                    mjs,
                    ret,
                    "actual arg #%d is not a string (the type idx is: %s)",
                    i,
                    mjs_typeof(arg));
                goto clean;
            }
            argvs[i] = arg;
            argvmgstr[i].p = mjs_get_string(mjs, &argvs[i], &argvmgstr[i].len);
            /*
         * String argument should be saved separately in order to support
         * short strings (which are packed into mjs_val_t itself)
         */
            ffi_set_ptr(&args[i], (void*)&argvmgstr[i]);
            break;
        }
        case MJS_FFI_CTYPE_BOOL: {
            int intval = 0;
            if(mjs_is_number(arg)) {
                intval = !!mjs_get_int(mjs, arg);
            } else if(mjs_is_boolean(arg)) {
                intval = mjs_get_bool(mjs, arg);
            } else {
                ret = MJS_TYPE_ERROR;
                mjs_prepend_errorf(
                    mjs,
                    ret,
                    "actual arg #%d is not a bool (the type idx is: %s)",
                    i,
                    mjs_typeof(arg));
            }
            ffi_set_word(&args[i], intval);
        } break;
        case MJS_FFI_CTYPE_DOUBLE:
            ffi_set_double(&args[i], mjs_get_double(mjs, arg));
            break;
        case MJS_FFI_CTYPE_FLOAT:
            ffi_set_float(&args[i], (float)mjs_get_double(mjs, arg));
            break;
        case MJS_FFI_CTYPE_CHAR_PTR: {
            size_t s;
            if(mjs_is_string(arg)) {
                /*
           * String argument should be saved separately in order to support
           * short strings (which are packed into mjs_val_t itself)
           */
                argvs[i] = arg;
                ffi_set_ptr(&args[i], (void*)mjs_get_string(mjs, &argvs[i], &s));
            } else if(mjs_is_null(arg)) {
                ffi_set_ptr(&args[i], NULL);
            } else {
                ret = MJS_TYPE_ERROR;
                mjs_prepend_errorf(
                    mjs,
                    ret,
                    "actual arg #%d is not a string (the type idx is: %s)",
                    i,
                    mjs_typeof(arg));
                goto clean;
            }
        } break;
        case MJS_FFI_CTYPE_VOID_PTR:
            if(mjs_is_string(arg)) {
                size_t n;
                /*
           * String argument should be saved separately in order to support
           * short strings (which are packed into mjs_val_t itself)
           */
                argvs[i] = arg;
                ffi_set_ptr(&args[i], (void*)mjs_get_string(mjs, &argvs[i], &n));
            } else if(mjs_is_foreign(arg)) {
                ffi_set_ptr(&args[i], (void*)mjs_get_ptr(mjs, arg));
            } else if(mjs_is_null(arg)) {
                ffi_set_ptr(&args[i], NULL);
            } else {
                ret = MJS_TYPE_ERROR;
                mjs_prepend_errorf(mjs, ret, "actual arg #%d is not a ptr", i);
                goto clean;
            }
            break;
        case MJS_FFI_CTYPE_CALLBACK:
            if(mjs_is_function(arg) || mjs_is_foreign(arg) || mjs_is_ffi_sig(arg)) {
                /*
           * Current argument is a callback function pointer: remember the given
           * JS function and the argument index
           */
                cbdata.func = arg;
                cbdata.func_idx = i;
            } else {
                ret = MJS_TYPE_ERROR;
                mjs_prepend_errorf(
                    mjs,
                    ret,
                    "actual arg #%d is not a function, but %s",
                    i,
                    mjs_stringify_type((enum mjs_type)arg));
                goto clean;
            }
            break;
        case MJS_FFI_CTYPE_INVALID:
            /* parse_cval_type() has already set a more detailed error */
            ret = MJS_TYPE_ERROR;
            mjs_prepend_errorf(mjs, ret, "wrong arg type");
            goto clean;
        default:
            abort();
            break;
        }
    }

    if(cbdata.userdata_idx >= 0 && cbdata.func_idx >= 0) {
        struct mjs_ffi_cb_args* cbargs = NULL;
        struct mjs_ffi_cb_args** pitem = NULL;

        /* the function takes a callback */

        /*
     * Get cbargs: either reuse the existing one (if the matching item exists),
     * or create a new one.
     */
        pitem = ffi_get_matching(&mjs->ffi_cb_args, cbdata.func, cbdata.userdata);
        if(*pitem == NULL) {
            /* No matching cbargs item; we need to add a new one */
            cbargs = calloc(1, sizeof(*cbargs));
            cbargs->mjs = mjs;
            cbargs->func = cbdata.func;
            cbargs->userdata = cbdata.userdata;
            mjs_ffi_sig_copy(&cbargs->sig, psig->cb_sig);

            /* Establish a link to the newly allocated item */
            *pitem = cbargs;
        } else {
            /* Found matching item: reuse it */
            cbargs = *pitem;
        }

        {
            union {
                ffi_fn_t* fn;
                void* p;
            } u;
            u.fn = psig->cb_sig->fn;
            ffi_set_ptr(&args[cbdata.func_idx], u.p);
            ffi_set_ptr(&args[cbdata.userdata_idx], cbargs);
        }
    } else if(!(cbdata.userdata_idx == -1 && cbdata.func_idx == -1)) {
        /*
     * incomplete signature: it contains either the function pointer or
     * userdata. It should contain both or none.
     *
     * It should be handled in mjs_parse_ffi_signature().
     */
        abort();
    }

    ffi_call_mjs(psig->fn, nargs, &res, args);

    switch(rtype) {
    case MJS_FFI_CTYPE_CHAR_PTR: {
        const char* s = (const char*)(uintptr_t)res.v.i;
        if(s != NULL) {
            resv = mjs_mk_string(mjs, s, ~0, 1);
        } else {
            resv = MJS_NULL;
        }
        break;
    }
    case MJS_FFI_CTYPE_VOID_PTR:
        resv = mjs_mk_foreign(mjs, (void*)(uintptr_t)res.v.i);
        break;
    case MJS_FFI_CTYPE_INT:
        resv = mjs_mk_number(mjs, (int)res.v.i);
        break;
    case MJS_FFI_CTYPE_BOOL:
        resv = mjs_mk_boolean(mjs, !!res.v.i);
        break;
    case MJS_FFI_CTYPE_DOUBLE:
        resv = mjs_mk_number(mjs, res.v.d);
        break;
    case MJS_FFI_CTYPE_FLOAT:
        resv = mjs_mk_number(mjs, res.v.f);
        break;
    default:
        resv = mjs_mk_undefined();
        break;
    }

clean:
    /*
   * If there was some error, prepend an error message with the subject
   * signature
   */
    if(ret != MJS_OK) {
        mjs_prepend_errorf(mjs, ret, "failed to call FFIed function");
        /* TODO(dfrank) stringify mjs_ffi_sig_t in some human-readable format */
    }
    mjs_return(mjs, resv);

    return ret;
}

/*
 * TODO(dfrank): make it return boolean (when booleans are supported), instead
 * of a number
 */
MJS_PRIVATE void mjs_ffi_cb_free(struct mjs* mjs) {
    mjs_val_t ret = mjs_mk_number(mjs, 0);
    mjs_val_t func = mjs_arg(mjs, 0);
    mjs_val_t userdata = mjs_arg(mjs, 1);

    if(mjs_is_function(func)) {
        struct mjs_ffi_cb_args** pitem = ffi_get_matching(&mjs->ffi_cb_args, func, userdata);
        if(*pitem != NULL) {
            /* Found matching item: remove it from the linked list, and free */
            struct mjs_ffi_cb_args* cbargs = *pitem;
            *pitem = cbargs->next;
            mjs_ffi_sig_free(&cbargs->sig);
            free(cbargs);
            ret = mjs_mk_number(mjs, 1);
        }
    } else {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "missing argument 'func'");
    }

    mjs_return(mjs, ret);
}

void mjs_ffi_args_free_list(struct mjs* mjs) {
    ffi_cb_args_t* next = mjs->ffi_cb_args;

    while(next != NULL) {
        ffi_cb_args_t* cur = next;
        next = next->next;
        free(cur);
    }
}

MJS_PRIVATE void mjs_ffi_sig_init(mjs_ffi_sig_t* sig) {
    memset(sig, 0, sizeof(*sig));
}

MJS_PRIVATE void mjs_ffi_sig_copy(mjs_ffi_sig_t* to, const mjs_ffi_sig_t* from) {
    memcpy(to, from, sizeof(*to));
    if(from->cb_sig != NULL) {
        to->cb_sig = calloc(sizeof(*to->cb_sig), 1);
        mjs_ffi_sig_copy(to->cb_sig, from->cb_sig);
    }
}

MJS_PRIVATE void mjs_ffi_sig_free(mjs_ffi_sig_t* sig) {
    if(sig->cb_sig != NULL) {
        free(sig->cb_sig);
        sig->cb_sig = NULL;
    }
}

MJS_PRIVATE int mjs_ffi_sig_set_val_type(mjs_ffi_sig_t* sig, int idx, mjs_ffi_ctype_t type) {
    if(idx < MJS_CB_SIGNATURE_MAX_SIZE) {
        sig->val_types[idx] = type;
        return 1;
    } else {
        /* Index is too large */
        return 0;
    }
}

MJS_PRIVATE int
    mjs_ffi_sig_validate(struct mjs* mjs, mjs_ffi_sig_t* sig, enum ffi_sig_type sig_type) {
    int ret = 0;
    int i;
    int callback_idx = 0;
    int userdata_idx = 0;

    sig->is_valid = 0;

    switch(sig_type) {
    case FFI_SIG_FUNC:
        /* Make sure return type is fine */
        if(sig->val_types[0] != MJS_FFI_CTYPE_NONE && sig->val_types[0] != MJS_FFI_CTYPE_INT &&
           sig->val_types[0] != MJS_FFI_CTYPE_BOOL && sig->val_types[0] != MJS_FFI_CTYPE_DOUBLE &&
           sig->val_types[0] != MJS_FFI_CTYPE_FLOAT &&
           sig->val_types[0] != MJS_FFI_CTYPE_VOID_PTR &&
           sig->val_types[0] != MJS_FFI_CTYPE_CHAR_PTR) {
            mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "invalid return value type");
            goto clean;
        }
        break;
    case FFI_SIG_CALLBACK:
        /* Make sure return type is fine */
        if(sig->val_types[0] != MJS_FFI_CTYPE_NONE && sig->val_types[0] != MJS_FFI_CTYPE_INT &&
           sig->val_types[0] != MJS_FFI_CTYPE_BOOL && sig->val_types[0] != MJS_FFI_CTYPE_DOUBLE &&
           sig->val_types[0] != MJS_FFI_CTYPE_FLOAT &&
           sig->val_types[0] != MJS_FFI_CTYPE_VOID_PTR) {
            mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "invalid return value type");
            goto clean;
        }
    }

    /* Handle argument types */
    for(i = 1; i < MJS_CB_SIGNATURE_MAX_SIZE; i++) {
        mjs_ffi_ctype_t type = sig->val_types[i];
        switch(type) {
        case MJS_FFI_CTYPE_USERDATA:
            if(userdata_idx != 0) {
                /* There must be at most one userdata arg, but we have more */
                mjs_prepend_errorf(
                    mjs,
                    MJS_TYPE_ERROR,
                    "more than one userdata arg: #%d and #%d",
                    (userdata_idx - 1),
                    (i - 1));
                goto clean;
            }
            userdata_idx = i;
            break;
        case MJS_FFI_CTYPE_CALLBACK:
            switch(sig_type) {
            case FFI_SIG_FUNC:
                break;
            case FFI_SIG_CALLBACK:
                mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "callback can't take another callback");
                goto clean;
            }
            callback_idx = i;
            break;
        case MJS_FFI_CTYPE_INT:
        case MJS_FFI_CTYPE_BOOL:
        case MJS_FFI_CTYPE_VOID_PTR:
        case MJS_FFI_CTYPE_CHAR_PTR:
        case MJS_FFI_CTYPE_STRUCT_MG_STR_PTR:
        case MJS_FFI_CTYPE_DOUBLE:
        case MJS_FFI_CTYPE_FLOAT:
            /* Do nothing */
            break;
        case MJS_FFI_CTYPE_NONE:
            /* No more arguments */
            goto args_over;
        default:
            mjs_prepend_errorf(mjs, MJS_INTERNAL_ERROR, "invalid ffi_ctype: %d", type);
            goto clean;
        }

        sig->args_cnt++;
    }
args_over:

    switch(sig_type) {
    case FFI_SIG_FUNC:
        if(!((callback_idx > 0 && userdata_idx > 0) || (callback_idx == 0 && userdata_idx == 0))) {
            mjs_prepend_errorf(
                mjs,
                MJS_TYPE_ERROR,
                "callback and userdata should be either both "
                "present or both absent");
            goto clean;
        }
        break;
    case FFI_SIG_CALLBACK:
        if(userdata_idx == 0) {
            /* No userdata arg */
            mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "no userdata arg");
            goto clean;
        }
        break;
    }

    ret = 1;

clean:
    if(ret) {
        sig->is_valid = 1;
    }
    return ret;
}

MJS_PRIVATE int mjs_ffi_is_regular_word(mjs_ffi_ctype_t type) {
    switch(type) {
    case MJS_FFI_CTYPE_INT:
    case MJS_FFI_CTYPE_BOOL:
        return 1;
    default:
        return 0;
    }
}

MJS_PRIVATE int mjs_ffi_is_regular_word_or_void(mjs_ffi_ctype_t type) {
    return (type == MJS_FFI_CTYPE_NONE || mjs_ffi_is_regular_word(type));
}

#ifdef _WIN32
void* dlsym(void* handle, const char* name) {
    static HANDLE msvcrt_dll;
    void* sym = NULL;
    if(msvcrt_dll == NULL) msvcrt_dll = GetModuleHandle("msvcrt.dll");
    if((sym = GetProcAddress(GetModuleHandle(NULL), name)) == NULL) {
        sym = GetProcAddress(msvcrt_dll, name);
    }
    return sym;
}
#elif !defined(__unix__) && !defined(__APPLE__)
void* dlsym(void* handle, const char* name) {
    (void)handle;
    (void)name;
    return NULL;
}
#endif
