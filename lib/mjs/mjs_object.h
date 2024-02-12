/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_OBJECT_H_
#define MJS_OBJECT_H_

#include "mjs_object_public.h"
#include "mjs_internal.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

struct mjs;

struct mjs_property {
    struct mjs_property* next; /* Linkage in struct mjs_object::properties */
    mjs_val_t name; /* Property name (a string) */
    mjs_val_t value; /* Property value */
};

struct mjs_object {
    struct mjs_property* properties;
};

MJS_PRIVATE struct mjs_object* get_object_struct(mjs_val_t v);
MJS_PRIVATE struct mjs_property*
    mjs_get_own_property(struct mjs* mjs, mjs_val_t obj, const char* name, size_t len);

MJS_PRIVATE struct mjs_property*
    mjs_get_own_property_v(struct mjs* mjs, mjs_val_t obj, mjs_val_t key);

/*
 * A worker function for `mjs_set()` and `mjs_set_v()`: it takes name as both
 * ptr+len and mjs_val_t. If `name` pointer is not NULL, it takes precedence
 * over `name_v`.
 */
MJS_PRIVATE mjs_err_t mjs_set_internal(
    struct mjs* mjs,
    mjs_val_t obj,
    mjs_val_t name_v,
    char* name,
    size_t name_len,
    mjs_val_t val);

/*
 * Implementation of `Object.create(proto)`
 */
MJS_PRIVATE void mjs_op_create_object(struct mjs* mjs);

#define MJS_PROTO_PROP_NAME "__p" /* Make it < 5 chars */

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_OBJECT_H_ */
