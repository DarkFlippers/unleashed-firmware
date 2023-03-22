#pragma once

#include "sha256.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hmac_context {
    void (*init_hash)(const struct hmac_context* context);
    void (*update_hash)(
        const struct hmac_context* context,
        const uint8_t* message,
        unsigned message_size);
    void (*finish_hash)(const struct hmac_context* context, uint8_t* hash_result);
    unsigned block_size; /* Hash function block size in bytes, eg 64 for SHA-256. */
    unsigned result_size; /* Hash function result size in bytes, eg 32 for SHA-256. */
    uint8_t* tmp; /* Must point to a buffer of at least (2 * result_size + block_size) bytes. */
} hmac_context;

typedef struct hmac_sha256_context {
    hmac_context hmac_ctx;
    sha256_context sha_ctx;
    uint8_t tmp[32 * 2 + 64];
} hmac_sha256_context;

void hmac_sha256_init(hmac_sha256_context* ctx, const uint8_t* K);

void hmac_sha256_update(
    const hmac_sha256_context* ctx,
    const uint8_t* message,
    unsigned message_size);

void hmac_sha256_finish(const hmac_sha256_context* ctx, const uint8_t* K, uint8_t* hash_result);

#ifdef __cplusplus
}
#endif
