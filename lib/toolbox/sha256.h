#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define SHA256_DIGEST_SIZE 32
#define SHA256_BLOCK_SIZE 64

typedef struct {
    uint32_t total[2];
    uint32_t state[8];
    uint32_t wbuf[16];
} sha256_context;

void sha256(const unsigned char* input, unsigned int ilen, unsigned char output[32]);
void sha256_start(sha256_context* ctx);
void sha256_finish(sha256_context* ctx, unsigned char output[32]);
void sha256_update(sha256_context* ctx, const unsigned char* input, unsigned int ilen);
void sha256_process(sha256_context* ctx);

#ifdef __cplusplus
}
#endif