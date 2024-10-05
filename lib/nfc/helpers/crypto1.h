#pragma once

#include <protocols/mf_classic/mf_classic.h>
#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t odd;
    uint32_t even;
} Crypto1;

Crypto1* crypto1_alloc(void);

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

uint32_t crypto1_lfsr_rollback_word(Crypto1* crypto1, uint32_t in, int fb);

bool crypto1_nonce_matches_encrypted_parity_bits(uint32_t nt, uint32_t ks, uint8_t nt_par_enc);

bool crypto1_is_weak_prng_nonce(uint32_t nonce);

uint32_t crypto1_decrypt_nt_enc(uint32_t cuid, uint32_t nt_enc, MfClassicKey known_key);

uint32_t crypto1_prng_successor(uint32_t x, uint32_t n);

#ifdef __cplusplus
}
#endif
