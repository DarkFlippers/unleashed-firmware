/*
 * Copyright (c) 2017 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_FFI_H_
#define MJS_FFI_H_

#include "ffi/ffi.h"
#include "mjs_ffi_public.h"
#include "mjs_internal.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

#define MJS_CB_ARGS_MAX_CNT 6
#define MJS_CB_SIGNATURE_MAX_SIZE (MJS_CB_ARGS_MAX_CNT + 1 /* return type */)

typedef uint8_t mjs_ffi_ctype_t;

enum ffi_sig_type {
    FFI_SIG_FUNC,
    FFI_SIG_CALLBACK,
};

/*
 * Parsed FFI signature
 */
struct mjs_ffi_sig {
    /*
   * Callback signature, corresponds to the arg of type MJS_FFI_CTYPE_CALLBACK
   * TODO(dfrank): probably we'll need to support multiple callback/userdata
   * pairs
   *
   * NOTE(dfrank): instances of this structure are grouped into GC arenas and
   * managed by GC, and for the GC mark to work, the first element should be
   * a pointer (so that the two LSBs are not used).
   */
    struct mjs_ffi_sig* cb_sig;

    /*
   * The first item is the return value type (for `void`, `MJS_FFI_CTYPE_NONE`
   * is used); the rest are arguments. If some argument is
   * `MJS_FFI_CTYPE_NONE`, it means that there are no more arguments.
   */
    mjs_ffi_ctype_t val_types[MJS_CB_SIGNATURE_MAX_SIZE];

    /*
   * Function to call. If `is_callback` is not set, then it's the function
   * obtained by dlsym; otherwise it's a pointer to the appropriate callback
   * implementation.
   */
    ffi_fn_t* fn;

    /* Number of arguments in the signature */
    int8_t args_cnt;

    /*
   * If set, then the signature represents the callback (as opposed to a normal
   * function), and `fn` points to the suitable callback implementation.
   */
    unsigned is_callback : 1;
    unsigned is_valid : 1;
};
typedef struct mjs_ffi_sig mjs_ffi_sig_t;

/* Initialize new FFI signature */
MJS_PRIVATE void mjs_ffi_sig_init(mjs_ffi_sig_t* sig);
/* Copy existing FFI signature */
MJS_PRIVATE void mjs_ffi_sig_copy(mjs_ffi_sig_t* to, const mjs_ffi_sig_t* from);
/* Free FFI signature. NOTE: the pointer `sig` itself is not freed */
MJS_PRIVATE void mjs_ffi_sig_free(mjs_ffi_sig_t* sig);

/*
 * Creates a new FFI signature from the GC arena, and return mjs_val_t which
 * wraps it.
 */
MJS_PRIVATE mjs_val_t mjs_mk_ffi_sig(struct mjs* mjs);

/*
 * Checks whether the given value is a FFI signature.
 */
MJS_PRIVATE int mjs_is_ffi_sig(mjs_val_t v);

/*
 * Wraps FFI signature structure into mjs_val_t value.
 */
MJS_PRIVATE mjs_val_t mjs_ffi_sig_to_value(struct mjs_ffi_sig* psig);

/*
 * Extracts a pointer to the FFI signature struct from the mjs_val_t value.
 */
MJS_PRIVATE struct mjs_ffi_sig* mjs_get_ffi_sig_struct(mjs_val_t v);

/*
 * A wrapper for mjs_ffi_sig_free() suitable to use as a GC cell destructor.
 */
MJS_PRIVATE void mjs_ffi_sig_destructor(struct mjs* mjs, void* psig);

MJS_PRIVATE int mjs_ffi_sig_set_val_type(mjs_ffi_sig_t* sig, int idx, mjs_ffi_ctype_t type);
MJS_PRIVATE int
    mjs_ffi_sig_validate(struct mjs* mjs, mjs_ffi_sig_t* sig, enum ffi_sig_type sig_type);
MJS_PRIVATE int mjs_ffi_is_regular_word(mjs_ffi_ctype_t type);
MJS_PRIVATE int mjs_ffi_is_regular_word_or_void(mjs_ffi_ctype_t type);

struct mjs_ffi_cb_args {
    struct mjs_ffi_cb_args* next;
    struct mjs* mjs;
    mjs_ffi_sig_t sig;
    mjs_val_t func;
    mjs_val_t userdata;
};
typedef struct mjs_ffi_cb_args ffi_cb_args_t;

/*
 * cfunction:
 * Parses the FFI signature string and returns a value wrapping mjs_ffi_sig_t.
 */
MJS_PRIVATE mjs_err_t mjs_ffi_call(struct mjs* mjs);

/*
 * cfunction:
 * Performs the FFI signature call.
 */
MJS_PRIVATE mjs_err_t mjs_ffi_call2(struct mjs* mjs);

MJS_PRIVATE void mjs_ffi_cb_free(struct mjs*);
MJS_PRIVATE void mjs_ffi_args_free_list(struct mjs* mjs);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_FFI_H_ */
