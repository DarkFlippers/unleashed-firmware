/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include <stdio.h>
#include "common/str_util.h"
#include "mjs_array.h"
#include "mjs_core.h"
#include "mjs_internal.h"
#include "mjs_object.h"
#include "mjs_primitive.h"
#include "mjs_string.h"
#include "mjs_util.h"

#define SPLICE_NEW_ITEM_IDX 2

/* like c_snprintf but returns `size` if write is truncated */
static int v_sprintf_s(char* buf, size_t size, const char* fmt, ...) {
    size_t n;
    va_list ap;

    va_start(ap, fmt);
    n = c_vsnprintf(buf, size, fmt, ap);
    va_end(ap);

    if(n > size) {
        return size;
    }
    return n;
}

mjs_val_t mjs_mk_array(struct mjs* mjs) {
    mjs_val_t ret = mjs_mk_object(mjs);
    /* change the tag to MJS_TAG_ARRAY */
    ret &= ~MJS_TAG_MASK;
    ret |= MJS_TAG_ARRAY;
    return ret;
}

int mjs_is_array(mjs_val_t v) {
    return (v & MJS_TAG_MASK) == MJS_TAG_ARRAY;
}

mjs_val_t mjs_array_get(struct mjs* mjs, mjs_val_t arr, unsigned long index) {
    return mjs_array_get2(mjs, arr, index, NULL);
}

mjs_val_t mjs_array_get2(struct mjs* mjs, mjs_val_t arr, unsigned long index, int* has) {
    mjs_val_t res = MJS_UNDEFINED;

    if(has != NULL) {
        *has = 0;
    }

    if(mjs_is_object(arr)) {
        struct mjs_property* p;
        char buf[20];
        int n = v_sprintf_s(buf, sizeof(buf), "%lu", index);
        p = mjs_get_own_property(mjs, arr, buf, n);
        if(p != NULL) {
            if(has != NULL) {
                *has = 1;
            }
            res = p->value;
        }
    }

    return res;
}

unsigned long mjs_array_length(struct mjs* mjs, mjs_val_t v) {
    struct mjs_property* p;
    unsigned long len = 0;

    if(!mjs_is_object(v)) {
        len = 0;
        goto clean;
    }

    for(p = get_object_struct(v)->properties; p != NULL; p = p->next) {
        int ok = 0;
        unsigned long n = 0;
        str_to_ulong(mjs, p->name, &ok, &n);
        if(ok && n >= len && n < 0xffffffff) {
            len = n + 1;
        }
    }

clean:
    return len;
}

mjs_err_t mjs_array_set(struct mjs* mjs, mjs_val_t arr, unsigned long index, mjs_val_t v) {
    mjs_err_t ret = MJS_OK;

    if(mjs_is_object(arr)) {
        char buf[20];
        int n = v_sprintf_s(buf, sizeof(buf), "%lu", index);
        ret = mjs_set(mjs, arr, buf, n, v);
    } else {
        ret = MJS_TYPE_ERROR;
    }

    return ret;
}

void mjs_array_del(struct mjs* mjs, mjs_val_t arr, unsigned long index) {
    char buf[20];
    int n = v_sprintf_s(buf, sizeof(buf), "%lu", index);
    mjs_del(mjs, arr, buf, n);
}

mjs_err_t mjs_array_push(struct mjs* mjs, mjs_val_t arr, mjs_val_t v) {
    return mjs_array_set(mjs, arr, mjs_array_length(mjs, arr), v);
}

MJS_PRIVATE void mjs_array_push_internal(struct mjs* mjs) {
    mjs_err_t rcode = MJS_OK;
    mjs_val_t ret = MJS_UNDEFINED;
    int nargs = mjs_nargs(mjs);
    int i;

    /* Make sure that `this` is an array */
    if(!mjs_check_arg(mjs, -1 /*this*/, "this", MJS_TYPE_OBJECT_ARRAY, NULL)) {
        goto clean;
    }

    /* Push all args */
    for(i = 0; i < nargs; i++) {
        rcode = mjs_array_push(mjs, mjs->vals.this_obj, mjs_arg(mjs, i));
        if(rcode != MJS_OK) {
            mjs_prepend_errorf(mjs, rcode, "");
            goto clean;
        }
    }

    /* Return the new array length */
    ret = mjs_mk_number(mjs, mjs_array_length(mjs, mjs->vals.this_obj));

clean:
    mjs_return(mjs, ret);
    return;
}

static void move_item(struct mjs* mjs, mjs_val_t arr, unsigned long from, unsigned long to) {
    mjs_val_t cur = mjs_array_get(mjs, arr, from);
    mjs_array_set(mjs, arr, to, cur);
    mjs_array_del(mjs, arr, from);
}

MJS_PRIVATE void mjs_array_splice(struct mjs* mjs) {
    int nargs = mjs_nargs(mjs);
    mjs_err_t rcode = MJS_OK;
    mjs_val_t ret = mjs_mk_array(mjs);
    mjs_val_t start_v = MJS_UNDEFINED;
    mjs_val_t deleteCount_v = MJS_UNDEFINED;
    int start = 0;
    int arr_len;
    int delete_cnt = 0;
    int new_items_cnt = 0;
    int delta = 0;
    int i;

    /* Make sure that `this` is an array */
    if(!mjs_check_arg(mjs, -1 /*this*/, "this", MJS_TYPE_OBJECT_ARRAY, NULL)) {
        goto clean;
    }

    /* Get array length */
    arr_len = mjs_array_length(mjs, mjs->vals.this_obj);

    /* get start from arg 0 */
    if(!mjs_check_arg(mjs, 0, "start", MJS_TYPE_NUMBER, &start_v)) {
        goto clean;
    }
    start = mjs_normalize_idx(mjs_get_int(mjs, start_v), arr_len);

    /* Handle deleteCount */
    if(nargs >= SPLICE_NEW_ITEM_IDX) {
        /* deleteCount is given; use it */
        if(!mjs_check_arg(mjs, 1, "deleteCount", MJS_TYPE_NUMBER, &deleteCount_v)) {
            goto clean;
        }
        delete_cnt = mjs_get_int(mjs, deleteCount_v);
        new_items_cnt = nargs - SPLICE_NEW_ITEM_IDX;
    } else {
        /* deleteCount is not given; assume the end of the array */
        delete_cnt = arr_len - start;
    }
    if(delete_cnt > arr_len - start) {
        delete_cnt = arr_len - start;
    } else if(delete_cnt < 0) {
        delete_cnt = 0;
    }

    /* delta at which subsequent array items should be moved */
    delta = new_items_cnt - delete_cnt;

    /*
   * copy items which are going to be deleted to the separate array (will be
   * returned)
   */
    for(i = 0; i < delete_cnt; i++) {
        mjs_val_t cur = mjs_array_get(mjs, mjs->vals.this_obj, start + i);
        rcode = mjs_array_push(mjs, ret, cur);
        if(rcode != MJS_OK) {
            mjs_prepend_errorf(mjs, rcode, "");
            goto clean;
        }
    }

    /* If needed, move subsequent items */
    if(delta < 0) {
        for(i = start; i < arr_len; i++) {
            if(i >= start - delta) {
                move_item(mjs, mjs->vals.this_obj, i, i + delta);
            } else {
                mjs_array_del(mjs, mjs->vals.this_obj, i);
            }
        }
    } else if(delta > 0) {
        for(i = arr_len - 1; i >= start; i--) {
            move_item(mjs, mjs->vals.this_obj, i, i + delta);
        }
    }

    /* Set new items to the array */
    for(i = 0; i < nargs - SPLICE_NEW_ITEM_IDX; i++) {
        mjs_array_set(mjs, mjs->vals.this_obj, start + i, mjs_arg(mjs, SPLICE_NEW_ITEM_IDX + i));
    }

clean:
    mjs_return(mjs, ret);
}
