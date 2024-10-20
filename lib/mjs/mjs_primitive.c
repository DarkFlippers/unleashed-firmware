/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#include "mjs_core.h"
#include "mjs_internal.h"
#include "mjs_primitive.h"
#include "mjs_string_public.h"
#include "mjs_util.h"

mjs_val_t mjs_mk_null(void) {
    return MJS_NULL;
}

int mjs_is_null(mjs_val_t v) {
    return v == MJS_NULL;
}

mjs_val_t mjs_mk_undefined(void) {
    return MJS_UNDEFINED;
}

int mjs_is_undefined(mjs_val_t v) {
    return v == MJS_UNDEFINED;
}

mjs_val_t mjs_mk_number(struct mjs* mjs, double v) {
    mjs_val_t res;
    (void)mjs;
    /* not every NaN is a JS NaN */
    if(isnan(v)) {
        res = MJS_TAG_NAN;
    } else {
        union {
            double d;
            mjs_val_t r;
        } u;
        u.d = v;
        res = u.r;
    }
    return res;
}

static double get_double(mjs_val_t v) {
    union {
        double d;
        mjs_val_t v;
    } u;
    u.v = v;
    /* Due to NaN packing, any non-numeric value is already a valid NaN value */
    return u.d;
}

double mjs_get_double(struct mjs* mjs, mjs_val_t v) {
    (void)mjs;
    return get_double(v);
}

int mjs_get_int(struct mjs* mjs, mjs_val_t v) {
    (void)mjs;
    /*
   * NOTE(dfrank): without double cast, all numbers >= 0x80000000 are always
   * converted to exactly 0x80000000.
   */
    return (int)(unsigned int)get_double(v);
}

int32_t mjs_get_int32(struct mjs* mjs, mjs_val_t v) {
    (void)mjs;
    return (int32_t)get_double(v);
}

int mjs_is_number(mjs_val_t v) {
    return v == MJS_TAG_NAN || !isnan(get_double(v));
}

mjs_val_t mjs_mk_boolean(struct mjs* mjs, int v) {
    (void)mjs;
    return (v ? 1 : 0) | MJS_TAG_BOOLEAN;
}

int mjs_get_bool(struct mjs* mjs, mjs_val_t v) {
    (void)mjs;
    if(mjs_is_boolean(v)) {
        return v & 1;
    } else {
        return 0;
    }
}

int mjs_is_boolean(mjs_val_t v) {
    return (v & MJS_TAG_MASK) == MJS_TAG_BOOLEAN;
}

#define MJS_IS_POINTER_LEGIT(n) \
    (((n)&MJS_TAG_MASK) == 0 || ((n)&MJS_TAG_MASK) == (~0 & MJS_TAG_MASK))

MJS_PRIVATE mjs_val_t mjs_pointer_to_value(struct mjs* mjs, void* p) {
    uint64_t n = ((uint64_t)(uintptr_t)p);

    if(!MJS_IS_POINTER_LEGIT(n)) {
        mjs_prepend_errorf(mjs, MJS_TYPE_ERROR, "invalid pointer value: %p", p);
    }
    return n & ~MJS_TAG_MASK;
}

MJS_PRIVATE mjs_val_t mjs_legit_pointer_to_value(void* p) {
    uint64_t n = ((uint64_t)(uintptr_t)p);

    assert(MJS_IS_POINTER_LEGIT(n));
    return n & ~MJS_TAG_MASK;
}

MJS_PRIVATE void* get_ptr(mjs_val_t v) {
    return (void*)(uintptr_t)(v & 0xFFFFFFFFFFFFUL);
}

void* mjs_get_ptr(struct mjs* mjs, mjs_val_t v) {
    (void)mjs;
    if(!mjs_is_foreign(v)) {
        return NULL;
    }
    return get_ptr(v);
}

mjs_val_t mjs_mk_foreign(struct mjs* mjs, void* p) {
    (void)mjs;
    return mjs_pointer_to_value(mjs, p) | MJS_TAG_FOREIGN;
}

mjs_val_t mjs_mk_foreign_func(struct mjs* mjs, mjs_func_ptr_t fn) {
    union {
        mjs_func_ptr_t fn;
        void* p;
    } u;
    u.fn = fn;
    (void)mjs;
    return mjs_pointer_to_value(mjs, u.p) | MJS_TAG_FOREIGN;
}

int mjs_is_foreign(mjs_val_t v) {
    return (v & MJS_TAG_MASK) == MJS_TAG_FOREIGN;
}

mjs_val_t mjs_mk_function(struct mjs* mjs, size_t off) {
    (void)mjs;
    return (mjs_val_t)off | MJS_TAG_FUNCTION;
}

int mjs_is_function(mjs_val_t v) {
    return (v & MJS_TAG_MASK) == MJS_TAG_FUNCTION;
}

MJS_PRIVATE void mjs_op_isnan(struct mjs* mjs) {
    mjs_val_t ret = MJS_UNDEFINED;
    mjs_val_t val = mjs_arg(mjs, 0);

    ret = mjs_mk_boolean(mjs, val == MJS_TAG_NAN);

    mjs_return(mjs, ret);
}

MJS_PRIVATE void mjs_number_to_string(struct mjs* mjs) {
    mjs_val_t ret = MJS_UNDEFINED;
    mjs_val_t base_v = MJS_UNDEFINED;
    int32_t base = 10;
    int32_t num;

    /* get number from `this` */
    if(!mjs_check_arg(mjs, -1 /*this*/, "this", MJS_TYPE_NUMBER, NULL)) {
        goto clean;
    }
    num = mjs_get_int32(mjs, mjs->vals.this_obj);

    if(mjs_nargs(mjs) >= 1) {
        /* get base from arg 0 */
        if(!mjs_check_arg(mjs, 0, "base", MJS_TYPE_NUMBER, &base_v)) {
            goto clean;
        }
        base = mjs_get_int(mjs, base_v);
    }

    char tmp_str[] = "-2147483648";
    itoa(num, tmp_str, base);
    ret = mjs_mk_string(mjs, tmp_str, ~0, true);

clean:
    mjs_return(mjs, ret);
}
