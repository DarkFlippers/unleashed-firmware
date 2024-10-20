/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_PRIMITIVE_H
#define MJS_PRIMITIVE_H

#include "mjs_primitive_public.h"
#include "mjs_internal.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*
 * Convert a pointer to mjs_val_t. If pointer is not valid, mjs crashes.
 */
MJS_PRIVATE mjs_val_t mjs_legit_pointer_to_value(void* p);

/*
 * Convert a pointer to mjs_val_t. If pointer is not valid, error is set
 * in the mjs context.
 */
MJS_PRIVATE mjs_val_t mjs_pointer_to_value(struct mjs* mjs, void* p);

/*
 * Extracts a pointer from the mjs_val_t value.
 */
MJS_PRIVATE void* get_ptr(mjs_val_t v);

/*
 * Implementation for JS isNaN()
 */
MJS_PRIVATE void mjs_op_isnan(struct mjs* mjs);

/*
 * Implementation for JS Number.toString()
 */
MJS_PRIVATE void mjs_number_to_string(struct mjs* mjs);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_PRIMITIVE_H */
