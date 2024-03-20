/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_CORE_PUBLIC_H_
#define MJS_CORE_PUBLIC_H_

#if !defined(_MSC_VER) || _MSC_VER >= 1700
#include <stdint.h>
#else
typedef unsigned __int64 uint64_t;
typedef int int32_t;
typedef unsigned char uint8_t;
#endif
#include <stdio.h>
#include <stddef.h>
#include "mjs_features.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#ifndef MJS_ENABLE_DEBUG
#define MJS_ENABLE_DEBUG 0
#endif

/*
 *  Double-precision floating-point number, IEEE 754
 *
 *  64 bit (8 bytes) in total
 *  1  bit sign
 *  11 bits exponent
 *  52 bits mantissa
 *      7         6        5        4        3        2        1        0
 *  seeeeeee|eeeemmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm|mmmmmmmm
 *
 * If an exponent is all-1 and mantissa is all-0, then it is an INFINITY:
 *  11111111|11110000|00000000|00000000|00000000|00000000|00000000|00000000
 *
 * If an exponent is all-1 and mantissa's MSB is 1, it is a quiet NaN:
 *  11111111|11111000|00000000|00000000|00000000|00000000|00000000|00000000
 *
 *  MJS NaN-packing:
 *    sign and exponent is 0xfff
 *    4 bits specify type (tag), must be non-zero
 *    48 bits specify value
 *
 *  11111111|1111tttt|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv|vvvvvvvv
 *   NaN marker |type|  48-bit placeholder for values: pointers, strings
 *
 * On 64-bit platforms, pointers are really 48 bit only, so they can fit,
 * provided they are sign extended
 */

typedef uint64_t mjs_val_t;

/*
 * A tag is made of the sign bit and the 4 lower order bits of byte 6.
 * So in total we have 32 possible tags.
 *
 * Tag (1,0) however cannot hold a zero payload otherwise it's interpreted as an
 * INFINITY; for simplicity we're just not going to use that combination.
 */
#define MAKE_TAG(s, t) ((uint64_t)(s) << 63 | (uint64_t)0x7ff0 << 48 | (uint64_t)(t) << 48)

#define MJS_TAG_OBJECT MAKE_TAG(1, 1)
#define MJS_TAG_FOREIGN MAKE_TAG(1, 2)
#define MJS_TAG_UNDEFINED MAKE_TAG(1, 3)
#define MJS_TAG_BOOLEAN MAKE_TAG(1, 4)
#define MJS_TAG_NAN MAKE_TAG(1, 5)
#define MJS_TAG_STRING_I MAKE_TAG(1, 6) /* Inlined string len < 5 */
#define MJS_TAG_STRING_5 MAKE_TAG(1, 7) /* Inlined string len 5 */
#define MJS_TAG_STRING_O MAKE_TAG(1, 8) /* Owned string */
#define MJS_TAG_STRING_F MAKE_TAG(1, 9) /* Foreign string */
#define MJS_TAG_STRING_C MAKE_TAG(1, 10) /* String chunk */
#define MJS_TAG_STRING_D MAKE_TAG(1, 11) /* Dictionary string  */
#define MJS_TAG_ARRAY MAKE_TAG(1, 12)
#define MJS_TAG_FUNCTION MAKE_TAG(1, 13)
#define MJS_TAG_FUNCTION_FFI MAKE_TAG(1, 14)
#define MJS_TAG_NULL MAKE_TAG(1, 15)

#define MJS_TAG_ARRAY_BUF MAKE_TAG(0, 1) /* ArrayBuffer */
#define MJS_TAG_ARRAY_BUF_VIEW MAKE_TAG(0, 2) /* DataView */

#define MJS_TAG_MASK MAKE_TAG(1, 15)

/* This if-0 is a dirty workaround to force etags to pick `struct mjs` */
#if 0
/* Opaque structure. MJS engine context. */
struct mjs {
  /* ... */
};
#endif

struct mjs;

enum mjs_type {
    /* Primitive types */
    MJS_TYPE_UNDEFINED,
    MJS_TYPE_NULL,
    MJS_TYPE_BOOLEAN,
    MJS_TYPE_NUMBER,
    MJS_TYPE_STRING,
    MJS_TYPE_FOREIGN,
    MJS_TYPE_ARRAY_BUF,
    MJS_TYPE_ARRAY_BUF_VIEW,

    /* Different classes of Object type */
    MJS_TYPE_OBJECT_GENERIC,
    MJS_TYPE_OBJECT_ARRAY,
    MJS_TYPE_OBJECT_FUNCTION,
    /*
   * TODO(dfrank): if we support prototypes, need to add items for them here
   */

    MJS_TYPES_CNT
};

typedef enum mjs_err {
    MJS_OK,
    MJS_SYNTAX_ERROR,
    MJS_REFERENCE_ERROR,
    MJS_TYPE_ERROR,
    MJS_OUT_OF_MEMORY,
    MJS_INTERNAL_ERROR,
    MJS_NOT_IMPLEMENTED_ERROR,
    MJS_FILE_READ_ERROR,
    MJS_BAD_ARGS_ERROR,

    MJS_NEED_EXIT,

    MJS_ERRS_CNT
} mjs_err_t;

typedef void (*mjs_flags_poller_t)(struct mjs* mjs);

struct mjs;

/* Create MJS instance */
struct mjs* mjs_create(void* context);

/* Destroy MJS instance */
void mjs_destroy(struct mjs* mjs);

mjs_val_t mjs_get_global(struct mjs* mjs);

/*
 * Tells the GC about an MJS value variable/field owned by C code.
 *
 * The user's C code should own mjs_val_t variables if the value's lifetime
 * crosses any invocation of `mjs_exec()` and friends, including `mjs_call()`.
 *
 * The registration of the variable prevents the GC from mistakenly treat the
 * object as garbage.
 *
 * User code should also explicitly disown the variables with `mjs_disown()`
 * once it goes out of scope or the structure containing the mjs_val_t field is
 * freed.
 *
 * Consider the following examples:
 *
 * Correct (owning is not necessary):
 * ```c
 * mjs_val_t res;
 * mjs_exec(mjs, "....some script", &res);
 * // ... use res somehow
 *
 * mjs_val_t res;
 * mjs_exec(mjs, "....some script2", &res);
 * // ... use new res somehow
 * ```
 *
 * WRONG:
 * ```c
 * mjs_val_t res1;
 * mjs_exec(mjs, "....some script", &res1);
 *
 * mjs_val_t res2;
 * mjs_exec(mjs, "....some script2", &res2);
 *
 * // ... use res1 (WRONG!) and res2
 * ```
 *
 * The code above is wrong, because after the second invocation of
 * `mjs_exec()`, the value of `res1` is invalidated.
 *
 * Correct (res1 is owned)
 * ```c
 * mjs_val_t res1 = MJS_UNDEFINED;
 * mjs_own(mjs, &res1);
 * mjs_exec(mjs, "....some script", &res1);
 *
 * mjs_val_t res2 = MJS_UNDEFINED;
 * mjs_exec(mjs, "....some script2", &res2);
 *
 * // ... use res1 and res2
 * mjs_disown(mjs, &res1);
 * ```
 *
 * NOTE that we explicly initialized `res1` to a valid value before owning it
 * (in this case, the value is `MJS_UNDEFINED`). Owning an uninitialized
 * variable is an undefined behaviour.
 *
 * Of course, it's not an error to own a variable even if it's not mandatory:
 * e.g. in the last example we could own both `res1` and `res2`. Probably it
 * would help us in the future, when we refactor the code so that `res2` has to
 * be owned, and we could forget to do that.
 *
 * Also, if the user code has some C function called from MJS, and in this C
 * function some MJS value (`mjs_val_t`) needs to be stored somewhere and to
 * stay alive after the C function has returned, it also needs to be properly
 * owned.
 */
void mjs_own(struct mjs* mjs, mjs_val_t* v);

/*
 * Disowns the value previously owned by `mjs_own()`.
 *
 * Returns 1 if value is found, 0 otherwise.
 */
int mjs_disown(struct mjs* mjs, mjs_val_t* v);

mjs_err_t mjs_set_errorf(struct mjs* mjs, mjs_err_t err, const char* fmt, ...);

void mjs_exit(struct mjs* mjs);

void mjs_set_exec_flags_poller(struct mjs* mjs, mjs_flags_poller_t poller);

void* mjs_get_context(struct mjs* mjs);

/*
 * If there is no error message already set, then it's equal to
 * `mjs_set_errorf()`.
 *
 * Otherwise, an old message gets prepended with the new one, followed by a
 * colon. (the previously set error code is kept)
 */
mjs_err_t mjs_prepend_errorf(struct mjs* mjs, mjs_err_t err, const char* fmt, ...);

/*
 * Print the last error details. If print_stack_trace is non-zero, also
 * print stack trace. `msg` is the message which gets prepended to the actual
 * error message, if it's NULL, then "MJS error" is used.
 */
void mjs_print_error(struct mjs* mjs, FILE* fp, const char* msg, int print_stack_trace);

/*
 * return a string representation of an error.
 * the error string might be overwritten by calls to `mjs_set_errorf`.
 */
const char* mjs_strerror(struct mjs* mjs, enum mjs_err err);

const char* mjs_get_stack_trace(struct mjs* mjs);

/*
 * Sets whether *.jsc files are generated when *.js file is executed. By
 * default it's 0.
 *
 * If either `MJS_GENERATE_JSC` or `CS_MMAP` is off, then this function has no
 * effect.
 */
void mjs_set_generate_jsc(struct mjs* mjs, int generate_jsc);

/*
 * When invoked from a cfunction, returns number of arguments passed to the
 * current JS function call.
 */
int mjs_nargs(struct mjs* mjs);

/*
 * When invoked from a cfunction, returns n-th argument to the current JS
 * function call.
 */
mjs_val_t mjs_arg(struct mjs* mjs, int n);

/*
 * Sets return value for the current JS function call.
 */
void mjs_return(struct mjs* mjs, mjs_val_t v);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_CORE_PUBLIC_H_ */
