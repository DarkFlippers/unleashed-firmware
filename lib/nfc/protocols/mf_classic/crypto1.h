#pragma once

#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t odd;
    uint32_t even;
} Crypto1;

Crypto1* crypto1_alloc();

void crypto1_free(Crypto1* instance);

void crypto1_reset(Crypto1* crypto1);

void crypto1_init(Crypto1* crypto1, uint64_t key);

uint8_t crypto1_bit(Crypto1* crypto1, uint8_t in, int is_encrypted);

uint8_t crypto1_byte(Crypto1* crypto1, uint8_t in, int is_encrypted);

uint32_t crypto1_word(Crypto1* crypto1, uint32_t in, int is_encrypted);

void crypto1_decrypt(Crypto1* crypto, const BitBuffer* buff, BitBuffer* out);

void crypto1_encrypt(Crypto1* crypto, uint8_t* keystream, const BitBuffer* buff, BitBuffer* out);

void crypto1_encrypt_reader_nonce(
    Crypto1* crypto,
    uint64_t key,
    uint32_t cuid,
    uint8_t* nt,
    uint8_t* nr,
    BitBuffer* out,
    bool is_nested);

uint32_t prng_successor(uint32_t x, uint32_t n);

#ifdef __cplusplus
}
#endif
