/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_ARRAY_H_
#define MJS_ARRAY_H_

#include "mjs_internal.h"
#include "mjs_array_public.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

MJS_PRIVATE mjs_val_t mjs_array_get2(struct mjs* mjs, mjs_val_t arr, unsigned long index, int* has);

MJS_PRIVATE void mjs_array_splice(struct mjs* mjs);

MJS_PRIVATE void mjs_array_push_internal(struct mjs* mjs);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_ARRAY_H_ */
