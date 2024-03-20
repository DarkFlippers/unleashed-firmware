/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_FFI_FFI_H_
#define MJS_FFI_FFI_H_

#include "../common/platform.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*
 * Maximum number of word-sized args to ffi-ed function. If at least one
 * of the args is double, only 2 args are allowed.
 */
#define FFI_MAX_ARGS_CNT 6

typedef void(ffi_fn_t)(void);

typedef intptr_t ffi_word_t;

enum ffi_ctype {
    FFI_CTYPE_WORD,
    FFI_CTYPE_BOOL,
    FFI_CTYPE_FLOAT,
    FFI_CTYPE_DOUBLE,
};

struct ffi_arg {
    enum ffi_ctype ctype;
    union {
        uint64_t i;
        double d;
        float f;
    } v;
};

int ffi_call_mjs(ffi_fn_t* func, int nargs, struct ffi_arg* res, struct ffi_arg* args);

void ffi_set_word(struct ffi_arg* arg, ffi_word_t v);
void ffi_set_bool(struct ffi_arg* arg, bool v);
void ffi_set_ptr(struct ffi_arg* arg, void* v);
void ffi_set_double(struct ffi_arg* arg, double v);
void ffi_set_float(struct ffi_arg* arg, float v);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_FFI_FFI_H_ */
