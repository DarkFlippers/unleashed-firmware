/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_FFI_PUBLIC_H_
#define MJS_FFI_PUBLIC_H_

#include "mjs_core_public.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

enum mjs_ffi_ctype {
    MJS_FFI_CTYPE_NONE,
    MJS_FFI_CTYPE_USERDATA,
    MJS_FFI_CTYPE_CALLBACK,
    MJS_FFI_CTYPE_INT,
    MJS_FFI_CTYPE_BOOL,
    MJS_FFI_CTYPE_DOUBLE,
    MJS_FFI_CTYPE_FLOAT,
    MJS_FFI_CTYPE_CHAR_PTR,
    MJS_FFI_CTYPE_VOID_PTR,
    MJS_FFI_CTYPE_STRUCT_MG_STR_PTR,
    MJS_FFI_CTYPE_STRUCT_MG_STR,
    MJS_FFI_CTYPE_INVALID,
};

typedef void*(mjs_ffi_resolver_t)(void* handle, const char* symbol);

void mjs_set_ffi_resolver(struct mjs* mjs, mjs_ffi_resolver_t* dlsym, void* handle);

void* mjs_ffi_resolve(struct mjs* mjs, const char* symbol);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_FFI_PUBLIC_H_ */
