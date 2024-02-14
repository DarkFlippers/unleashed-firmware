/*
 * Copyright (c) 2016 Cesanta Software Limited
 * All rights reserved
 */

#ifndef MJS_JSON_H_
#define MJS_JSON_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

MJS_PRIVATE mjs_err_t to_json_or_debug(
    struct mjs* mjs,
    mjs_val_t v,
    char* buf,
    size_t size,
    size_t* res_len,
    uint8_t is_debug);

MJS_PRIVATE mjs_err_t
    mjs_json_stringify(struct mjs* mjs, mjs_val_t v, char* buf, size_t size, char** res);
MJS_PRIVATE void mjs_op_json_stringify(struct mjs* mjs);
MJS_PRIVATE void mjs_op_json_parse(struct mjs* mjs);

MJS_PRIVATE mjs_err_t mjs_json_parse(struct mjs* mjs, const char* str, size_t len, mjs_val_t* res);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* MJS_JSON_H_ */
