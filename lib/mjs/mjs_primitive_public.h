/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_PRIMITIVE_PUBLIC_H_
#define MJS_PRIMITIVE_PUBLIC_H_

#include "mjs_core_public.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* JavaScript `null` value */
#define MJS_NULL MJS_TAG_NULL

/* JavaScript `undefined` value */
#define MJS_UNDEFINED MJS_TAG_UNDEFINED

#define MJS_MK_FN(fn) mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)fn)

/* Function pointer type used in `mjs_mk_foreign_func`. */
typedef void (*mjs_func_ptr_t)(void);

/*
 * Make `null` primitive value.
 *
 * NOTE: this function is deprecated and will be removed in future releases.
 * Use `MJS_NULL` instead.
 */
mjs_val_t mjs_mk_null(void);

/* Returns true if given value is a primitive `null` value */
int mjs_is_null(mjs_val_t v);

/*
 * Make `undefined` primitive value.
 *
 * NOTE: this function is deprecated and will be removed in future releases.
 * Use `MJS_UNDEFINED` instead.
 */
mjs_val_t mjs_mk_undefined(void);

/* Returns true if given value is a primitive `undefined` value */
int mjs_is_undefined(mjs_val_t v);

/* Make numeric primitive value */
mjs_val_t mjs_mk_number(struct mjs* mjs, double num);

/*
 * Returns number value stored in `mjs_val_t` as `double`.
 *
 * Returns NaN for non-numbers.
 */
double mjs_get_double(struct mjs* mjs, mjs_val_t v);

/*
 * Returns number value stored in `mjs_val_t` as `int`. If the number value is
 * not an integer, the fraction part will be discarded.
 *
 * If the given value is a non-number, or NaN, the result is undefined.
 */
int mjs_get_int(struct mjs* mjs, mjs_val_t v);

/*
 * Like mjs_get_int but ensures that the returned type
 * is a 32-bit signed integer.
 */
int32_t mjs_get_int32(struct mjs* mjs, mjs_val_t v);

/* Returns true if given value is a primitive number value */
int mjs_is_number(mjs_val_t v);

/*
 * Make JavaScript value that holds C/C++ `void *` pointer.
 *
 * A foreign value is completely opaque and JS code cannot do anything useful
 * with it except holding it in properties and passing it around.
 * It behaves like a sealed object with no properties.
 *
 * NOTE:
 * Only valid pointers (as defined by each supported architecture) will fully
 * preserved. In particular, all supported 64-bit architectures (x86_64, ARM-64)
 * actually define a 48-bit virtual address space.
 * Foreign values will be sign-extended as required, i.e creating a foreign
 * value of something like `(void *) -1` will work as expected. This is
 * important because in some 64-bit OSs (e.g. Solaris) the user stack grows
 * downwards from the end of the address space.
 *
 * If you need to store exactly sizeof(void*) bytes of raw data where
 * `sizeof(void*)` >= 8, please use byte arrays instead.
 */
mjs_val_t mjs_mk_foreign(struct mjs* mjs, void* ptr);

/*
 * Make JavaScript value that holds C/C++ function pointer, similarly to
 * `mjs_mk_foreign`.
 */
mjs_val_t mjs_mk_foreign_func(struct mjs* mjs, mjs_func_ptr_t fn);

/*
 * Returns `void *` pointer stored in `mjs_val_t`.
 *
 * Returns NULL if the value is not a foreign pointer.
 */
void* mjs_get_ptr(struct mjs* mjs, mjs_val_t v);

/* Returns true if given value holds `void *` pointer */
int mjs_is_foreign(mjs_val_t v);

mjs_val_t mjs_mk_boolean(struct mjs* mjs, int v);
int mjs_get_bool(struct mjs* mjs, mjs_val_t v);
int mjs_is_boolean(mjs_val_t v);

mjs_val_t mjs_mk_function(struct mjs* mjs, size_t off);
int mjs_is_function(mjs_val_t v);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_PRIMITIVE_PUBLIC_H_ */
