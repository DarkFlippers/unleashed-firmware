/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_BUILTIN_H_
#define MJS_BUILTIN_H_

#include "mjs_core_public.h"
#include "mjs_internal.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

void mjs_init_builtin(struct mjs* mjs, mjs_val_t obj);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_BUILTIN_H_ */
