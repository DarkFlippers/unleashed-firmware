/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include "mjs_bcode.h"
#include "mjs_core.h"
#include "mjs_dataview.h"
#include "mjs_exec.h"
#include "mjs_gc.h"
#include "mjs_internal.h"
#include "mjs_json.h"
#include "mjs_object.h"
#include "mjs_primitive.h"
#include "mjs_string.h"
#include "mjs_util.h"
#include "mjs_array_buf.h"

/*
 * If the file with the given filename was already loaded, returns the
 * corresponding bcode part; otherwise returns NULL.
 */
static struct mjs_bcode_part* mjs_get_loaded_file_bcode(struct mjs* mjs, const char* filename) {
    int parts_cnt = mjs_bcode_parts_cnt(mjs);
    int i;

    if(filename == NULL) {
        return 0;
    }

    for(i = 0; i < parts_cnt; i++) {
        struct mjs_bcode_part* bp = mjs_bcode_part_get(mjs, i);
        const char* cur_fn = mjs_get_bcode_filename(mjs, bp);
        if(strcmp(filename, cur_fn) == 0) {
            return bp;
        }
    }
    return NULL;
}

static void mjs_load(struct mjs* mjs) {
    mjs_val_t res = MJS_UNDEFINED;
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    mjs_val_t arg1 = mjs_arg(mjs, 1);
    int custom_global = 0; /* whether the custom global object was provided */

    if(mjs_is_string(arg0)) {
        const char* path = mjs_get_cstring(mjs, &arg0);
        struct mjs_bcode_part* bp = NULL;
        mjs_err_t ret;

        if(mjs_is_object(arg1)) {
            custom_global = 1;
            push_mjs_val(&mjs->scopes, arg1);
        }
        bp = mjs_get_loaded_file_bcode(mjs, path);
        if(bp == NULL) {
            /* File was not loaded before, so, load */
            ret = mjs_exec_file(mjs, path, &res);
        } else {
            /*
       * File was already loaded before, so if it was evaluated successfully,
       * then skip the evaluation at all (and assume MJS_OK); otherwise
       * re-evaluate it again.
       *
       * However, if the custom global object was provided, then reevaluate
       * the file in any case.
       */
            if(bp->exec_res != MJS_OK || custom_global) {
                ret = mjs_execute(mjs, bp->start_idx, &res);
            } else {
                ret = MJS_OK;
            }
        }
        if(ret != MJS_OK) {
            /*
       * arg0 and path might be invalidated by executing a file, so refresh
       * them
       */
            arg0 = mjs_arg(mjs, 0);
            path = mjs_get_cstring(mjs, &arg0);
            mjs_prepend_errorf(mjs, ret, "failed to exec file \"%s\"", path);
            goto clean;
        }

    clean:
        if(custom_global) {
            mjs_pop_val(&mjs->scopes);
        }
    }
    mjs_return(mjs, res);
}

static void mjs_get_mjs(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_foreign(mjs, mjs));
}

static void mjs_chr(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0), res = MJS_NULL;
    int n = mjs_get_int(mjs, arg0);
    if(mjs_is_number(arg0) && n >= 0 && n <= 255) {
        uint8_t s = n;
        res = mjs_mk_string(mjs, (const char*)&s, sizeof(s), 1);
    }
    mjs_return(mjs, res);
}

static void mjs_do_gc(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    mjs_gc(mjs, mjs_is_boolean(arg0) ? mjs_get_bool(mjs, arg0) : 0);
    mjs_return(mjs, arg0);
}

static void mjs_s2o(struct mjs* mjs) {
    mjs_return(
        mjs,
        mjs_struct_to_obj(
            mjs,
            mjs_get_ptr(mjs, mjs_arg(mjs, 0)),
            (const struct mjs_c_struct_member*)mjs_get_ptr(mjs, mjs_arg(mjs, 1))));
}

void mjs_init_builtin(struct mjs* mjs, mjs_val_t obj) {
    mjs_val_t v;

    mjs_set(mjs, obj, "global", ~0, obj);

    mjs_set(mjs, obj, "load", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_load));
    mjs_set(mjs, obj, "ffi", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_ffi_call));
    mjs_set(
        mjs, obj, "ffi_cb_free", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_ffi_cb_free));
    mjs_set(mjs, obj, "mkstr", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_mkstr));
    mjs_set(mjs, obj, "getMJS", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_get_mjs));
    mjs_set(mjs, obj, "die", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_die));
    mjs_set(mjs, obj, "gc", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_do_gc));
    mjs_set(mjs, obj, "chr", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_chr));
    mjs_set(mjs, obj, "s2o", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_s2o));

    /*
    * Populate JSON.parse() and JSON.stringify()
    */
    // v = mjs_mk_object(mjs);
    // mjs_set(
    //     mjs, v, "stringify", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_op_json_stringify));
    // mjs_set(mjs, v, "parse", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_op_json_parse));
    // mjs_set(mjs, obj, "JSON", ~0, v);

    /*
    * Populate Object.create()
    */
    v = mjs_mk_object(mjs);
    mjs_set(mjs, v, "create", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_op_create_object));
    mjs_set(mjs, obj, "Object", ~0, v);

    /*
    * Populate numeric stuff
    */
    mjs_set(mjs, obj, "NaN", ~0, MJS_TAG_NAN);
    mjs_set(mjs, obj, "isNaN", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_op_isnan));

    mjs_init_builtin_array_buf(mjs, obj);
}
