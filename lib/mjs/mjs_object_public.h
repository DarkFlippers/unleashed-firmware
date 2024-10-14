/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_OBJECT_PUBLIC_H_
#define MJS_OBJECT_PUBLIC_H_

#include <stddef.h>
#include "mjs_core_public.h"
#include "mjs_ffi_public.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*
 * Returns true if the given value is an object or array.
 */
int mjs_is_object(mjs_val_t v);

/*
 * Returns true if the given value type is object-based (object, array, dataview).
 */
int mjs_is_object_based(mjs_val_t v);

/* Make an empty object */
mjs_val_t mjs_mk_object(struct mjs* mjs);

/* Field types for struct-object conversion. */
enum mjs_struct_field_type {
    MJS_STRUCT_FIELD_TYPE_INVALID,
    MJS_STRUCT_FIELD_TYPE_STRUCT, /* Struct, arg points to def. */
    MJS_STRUCT_FIELD_TYPE_STRUCT_PTR, /* Ptr to struct, arg points to def. */
    MJS_STRUCT_FIELD_TYPE_INT,
    MJS_STRUCT_FIELD_TYPE_BOOL,
    MJS_STRUCT_FIELD_TYPE_DOUBLE,
    MJS_STRUCT_FIELD_TYPE_FLOAT,
    MJS_STRUCT_FIELD_TYPE_CHAR_PTR, /* NUL-terminated string. */
    MJS_STRUCT_FIELD_TYPE_VOID_PTR, /* Converted to foreign ptr. */
    MJS_STRUCT_FIELD_TYPE_MG_STR_PTR, /* Converted to string. */
    MJS_STRUCT_FIELD_TYPE_MG_STR, /* Converted to string. */
    MJS_STRUCT_FIELD_TYPE_DATA, /* Data, arg is length, becomes string. */
    MJS_STRUCT_FIELD_TYPE_INT8,
    MJS_STRUCT_FIELD_TYPE_INT16,
    MJS_STRUCT_FIELD_TYPE_UINT8,
    MJS_STRUCT_FIELD_TYPE_UINT16,
    /*
   * User-provided function. Arg is a pointer to function that takes void *
   * (pointer to field within the struct) and returns mjs_val_t:
   * mjs_val_t field_value(struct mjs *mjs, const void *field_ptr) { ... }
   */
    MJS_STRUCT_FIELD_TYPE_CUSTOM,
};

/* C structure layout descriptor - needed by mjs_struct_to_obj */
struct mjs_c_struct_member {
    const char* name;
    int offset;
    enum mjs_struct_field_type type;
    const void* arg; /* Additional argument, used for some types. */
};

/* Create flat JS object from a C memory descriptor */
mjs_val_t
    mjs_struct_to_obj(struct mjs* mjs, const void* base, const struct mjs_c_struct_member* members);

/*
 * Lookup property `name` in object `obj`. If `obj` holds no such property,
 * an `undefined` value is returned.
 *
 * If `name_len` is ~0, `name` is assumed to be NUL-terminated and
 * `strlen(name)` is used.
 */
mjs_val_t mjs_get(struct mjs* mjs, mjs_val_t obj, const char* name, size_t name_len);

/*
 * Like mjs_get but with a JS string.
 */
mjs_val_t mjs_get_v(struct mjs* mjs, mjs_val_t obj, mjs_val_t name);

/*
 * Like mjs_get_v but lookup the prototype chain.
 */
mjs_val_t mjs_get_v_proto(struct mjs* mjs, mjs_val_t obj, mjs_val_t key);

/*
 * Set object property. Behaves just like JavaScript assignment.
 */
mjs_err_t mjs_set(struct mjs* mjs, mjs_val_t obj, const char* name, size_t len, mjs_val_t val);

/*
 * Like mjs_set but the name is already a JS string.
 */
mjs_err_t mjs_set_v(struct mjs* mjs, mjs_val_t obj, mjs_val_t name, mjs_val_t val);

/*
 * Delete own property `name` of the object `obj`. Does not follow the
 * prototype chain.
 *
 * If `name_len` is ~0, `name` is assumed to be NUL-terminated and
 * `strlen(name)` is used.
 *
 * Returns 0 on success, -1 on error.
 */
int mjs_del(struct mjs* mjs, mjs_val_t obj, const char* name, size_t len);

/*
 * Iterate over `obj` properties.
 * First call should set `iterator` to MJS_UNDEFINED.
 * Return object's key (a string), or MJS_UNDEFINED when no more keys left.
 * Do not mutate the object during iteration.
 *
 * Example:
 *   mjs_val_t key, iter = MJS_UNDEFINED;
 *   while ((key = mjs_next(mjs, obj, &iter)) != MJS_UNDEFINED) {
 *     // Do something with the obj/key ...
 *   }
 */
mjs_val_t mjs_next(struct mjs* mjs, mjs_val_t obj, mjs_val_t* iterator);

typedef void (*mjs_custom_obj_destructor_t)(struct mjs* mjs, mjs_val_t object);

/*
 * Destructor property name. If set, must be a foreign pointer to a function
 * that will be called just before the object is freed.
 */
#define MJS_DESTRUCTOR_PROP_NAME "__d"

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_OBJECT_PUBLIC_H_ */
