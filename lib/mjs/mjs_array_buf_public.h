/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#pragma once

#include "mjs_core_public.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

typedef enum {
    MJS_DATAVIEW_U8,
    MJS_DATAVIEW_I8,
    MJS_DATAVIEW_U16,
    MJS_DATAVIEW_I16,
    MJS_DATAVIEW_U32,
    MJS_DATAVIEW_I32,
} mjs_dataview_type_t;

int mjs_is_array_buf(mjs_val_t v);

int mjs_is_data_view(mjs_val_t v);

int mjs_is_typed_array(mjs_val_t v);

mjs_val_t mjs_mk_array_buf(struct mjs* mjs, char* data, size_t buf_len);

char* mjs_array_buf_get_ptr(struct mjs* mjs, mjs_val_t buf, size_t* bytelen);

mjs_val_t mjs_dataview_get_buf(struct mjs* mjs, mjs_val_t obj);

#if defined(__cplusplus)
}
#endif /* __cplusplus */
