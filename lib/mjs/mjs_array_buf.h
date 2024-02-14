/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#pragma once

#include "mjs_internal.h"
#include "mjs_array_buf_public.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

mjs_val_t mjs_dataview_get_prop(struct mjs* mjs, mjs_val_t obj, mjs_val_t key);

mjs_err_t mjs_dataview_set_prop(struct mjs* mjs, mjs_val_t obj, mjs_val_t key, mjs_val_t val);

void mjs_init_builtin_array_buf(struct mjs* mjs, mjs_val_t obj);

mjs_val_t mjs_dataview_get_len(struct mjs* mjs, mjs_val_t obj);

void mjs_array_buf_slice(struct mjs* mjs);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
