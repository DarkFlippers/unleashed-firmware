/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_UTIL_PUBLIC_H_
#define MJS_UTIL_PUBLIC_H_

#include "mjs_core_public.h"
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

typedef void (*MjsPrintCallback)(void* ctx, const char* format, ...);

const char* mjs_typeof(mjs_val_t v);

void mjs_fprintf(mjs_val_t v, struct mjs* mjs, FILE* fp);
void mjs_sprintf(mjs_val_t v, struct mjs* mjs, char* buf, size_t buflen);

void mjs_disasm_all(struct mjs* mjs, MjsPrintCallback print_cb, void* print_ctx);
void mjs_dump(struct mjs* mjs, int do_disasm, MjsPrintCallback print_cb, void* print_ctx);

/*
 * Returns the filename corresponding to the given bcode offset.
 */
const char* mjs_get_bcode_filename_by_offset(struct mjs* mjs, int offset);

/*
 * Returns the line number corresponding to the given bcode offset.
 */
int mjs_get_lineno_by_offset(struct mjs* mjs, int offset);

/*
 * Returns bcode offset of the corresponding call frame cf_num, where 0 means
 * the currently executing function, 1 means the first return address, etc.
 *
 * If given cf_num is too large, -1 is returned.
 */
int mjs_get_offset_by_call_frame_num(struct mjs* mjs, int cf_num);

/*
 * Tries to convert `mjs_val_t` to a string, returns MJS_OK if successful.
 * String is returned as a pair of pointers: `char **p, size_t *sizep`.
 *
 * Caller must also provide a non-null `need_free`, and if it is non-zero,
 * then the string `*p` should be freed by the caller.
 *
 * MJS does not support `toString()` and `valueOf()`, so, passing an object
 * always results in `MJS_TYPE_ERROR`.
 */
mjs_err_t mjs_to_string(struct mjs* mjs, mjs_val_t* v, char** p, size_t* sizep, int* need_free);

/*
 * Converts value to boolean as in the expression `if (v)`.
 */
mjs_val_t mjs_to_boolean_v(struct mjs* mjs, mjs_val_t v);

int mjs_is_truthy(struct mjs* mjs, mjs_val_t v);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_UTIL_PUBLIC_H_ */
