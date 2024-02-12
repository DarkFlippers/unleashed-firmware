/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_EXEC_H_
#define MJS_EXEC_H_

#include "mjs_exec_public.h"

/*
 * A special bcode offset value which causes mjs_execute() to exit immediately;
 * used in mjs_apply().
 */
#define MJS_BCODE_OFFSET_EXIT ((size_t)0x7fffffff)

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

MJS_PRIVATE mjs_err_t mjs_execute(struct mjs* mjs, size_t off, mjs_val_t* res);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_EXEC_H_ */
