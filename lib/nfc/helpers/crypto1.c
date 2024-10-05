#include "crypto1.h"

#include <lib/nfc/helpers/nfc_util.h>
#include <lib/bit_lib/bit_lib.h>
#include <furi.h>

// Algorithm from https://github.com/RfidResearchGroup/proxmark3.git

#define SWAPENDIAN(x) \
    ((x) = ((x) >> 8 & 0xff00ff) | ((x) & 0xff00ff) << 8, (x) = (x) >> 16 | (x) << 16)
#define LF_POLY_ODD  (0x29CE5C)
#define LF_POLY_EVEN (0x870804)

#define BEBIT(x, n) FURI_BIT(x, (n) ^ 24)

Crypto1* crypto1_alloc(void) {
    Crypto1* instance = malloc(sizeof(Crypto1));

    return instance;
}

void crypto1_free(Crypto1* instance) {
    furi_assert(instance);

    free(instance);
}

void crypto1_reset(Crypto1* crypto1) {
    furi_assert(crypto1);
    crypto1->even = 0;
    crypto1->odd = 0;
}

void crypto1_init(Crypto1* crypto1, uint64_t key) {
    furi_assert(crypto1);
    crypto1->even = 0;
    crypto1->odd = 0;
    for(int8_t i = 47; i > 0; i -= 2) {
        crypto1->odd = crypto1->odd << 1 | FURI_BIT(key, (i - 1) ^ 7);
        crypto1->even = crypto1->even << 1 | FURI_BIT(key, i ^ 7);
    }
}

static uint32_t crypto1_filter(uint32_t in) {
    uint32_t out = 0;
    out = 0xf22c0 >> (in & 0xf) & 16;
    out |= 0x6c9c0 >> (in >> 4 & 0xf) & 8;
    out |= 0x3c8b0 >> (in >> 8 & 0xf) & 4;
    out |= 0x1e458 >> (in >> 12 & 0xf) & 2;
    out |= 0x0d938 >> (in >> 16 & 0xf) & 1;
    return FURI_BIT(0xEC57E80A, out);
}

uint8_t crypto1_bit(Crypto1* crypto1, uint8_t in, int is_encrypted) {
    furi_assert(crypto1);
    uint8_t out = crypto1_filter(crypto1->odd);
    uint32_t feed = out & (!!is_encrypted);
    feed ^= !!in;
    feed ^= LF_POLY_ODD & crypto1->odd;
    feed ^= LF_POLY_EVEN & crypto1->even;
    crypto1->even = crypto1->even << 1 | (nfc_util_even_parity32(feed));

    FURI_SWAP(crypto1->odd, crypto1->even);
    return out;
}

uint8_t crypto1_byte(Crypto1* crypto1, uint8_t in, int is_encrypted) {
    furi_assert(crypto1);
    uint8_t out = 0;
    for(uint8_t i = 0; i < 8; i++) {
        out |= crypto1_bit(crypto1, FURI_BIT(in, i), is_encrypted) << i;
    }
    return out;
}

uint32_t crypto1_word(Crypto1* crypto1, uint32_t in, int is_encrypted) {
    furi_assert(crypto1);
    uint32_t out = 0;
    for(uint8_t i = 0; i < 32; i++) {
        out |= (uint32_t)crypto1_bit(crypto1, BEBIT(in, i), is_encrypted) << (24 ^ i);
    }
    return out;
}

uint32_t crypto1_prng_successor(uint32_t x, uint32_t n) {
    SWAPENDIAN(x);
    while(n--)
        x = x >> 1 | (x >> 16 ^ x >> 18 ^ x >> 19 ^ x >> 21) << 31;

    return SWAPENDIAN(x);
}

void crypto1_decrypt(Crypto1* crypto, const BitBuffer* buff, BitBuffer* out) {
    furi_assert(crypto);
    furi_assert(buff);
    furi_assert(out);

    size_t bits = bit_buffer_get_size(buff);
    bit_buffer_set_size(out, bits);
    const uint8_t* encrypted_data = bit_buffer_get_data(buff);
    if(bits < 8) {
        uint8_t decrypted_byte = 0;
        uint8_t encrypted_byte = encrypted_data[0];
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_byte, 0)) << 0;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_byte, 1)) << 1;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_byte, 2)) << 2;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_byte, 3)) << 3;
        bit_buffer_set_byte(out, 0, decrypted_byte);
    } else {
        for(size_t i = 0; i < bits / 8; i++) {
            uint8_t decrypted_byte = crypto1_byte(crypto, 0, 0) ^ encrypted_data[i];
            bit_buffer_set_byte(out, i, decrypted_byte);
        }
    }
}

void crypto1_encrypt(Crypto1* crypto, uint8_t* keystream, const BitBuffer* buff, BitBuffer* out) {
    furi_assert(crypto);
    furi_assert(buff);
    furi_assert(out);

    size_t bits = bit_buffer_get_size(buff);
    bit_buffer_set_size(out, bits);
    const uint8_t* plain_data = bit_buffer_get_data(buff);
    if(bits < 8) {
        uint8_t encrypted_byte = 0;
        for(size_t i = 0; i < bits; i++) {
            encrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(plain_data[0], i)) << i;
        }
        bit_buffer_set_byte(out, 0, encrypted_byte);
    } else {
        for(size_t i = 0; i < bits / 8; i++) {
            uint8_t encrypted_byte = crypto1_byte(crypto, keystream ? keystream[i] : 0, 0) ^
                                     plain_data[i];
            bool parity_bit =
                ((crypto1_filter(crypto->odd) ^ nfc_util_odd_parity8(plain_data[i])) & 0x01);
            bit_buffer_set_byte_with_parity(out, i, encrypted_byte, parity_bit);
        }
    }
}

void crypto1_encrypt_reader_nonce(
    Crypto1* crypto,
    uint64_t key,
    uint32_t cuid,
    uint8_t* nt,
    uint8_t* nr,
    BitBuffer* out,
    bool is_nested) {
    furi_assert(crypto);
    furi_assert(nt);
    furi_assert(nr);
    furi_assert(out);

    bit_buffer_set_size_bytes(out, 8);
    uint32_t nt_num = bit_lib_bytes_to_num_be(nt, sizeof(uint32_t));

    crypto1_init(crypto, key);
    if(is_nested) {
        nt_num = crypto1_word(crypto, nt_num ^ cuid, 1) ^ nt_num;
    } else {
        crypto1_word(crypto, nt_num ^ cuid, 0);
    }

    for(size_t i = 0; i < 4; i++) {
        uint8_t byte = crypto1_byte(crypto, nr[i], 0) ^ nr[i];
        bool parity_bit = ((crypto1_filter(crypto->odd) ^ nfc_util_odd_parity8(nr[i])) & 0x01);
        bit_buffer_set_byte_with_parity(out, i, byte, parity_bit);
        nr[i] = byte;
    }

    nt_num = crypto1_prng_successor(nt_num, 32);
    for(size_t i = 4; i < 8; i++) {
        nt_num = crypto1_prng_successor(nt_num, 8);
        uint8_t byte = crypto1_byte(crypto, 0, 0) ^ (uint8_t)(nt_num);
        bool parity_bit = ((crypto1_filter(crypto->odd) ^ nfc_util_odd_parity8(nt_num)) & 0x01);
        bit_buffer_set_byte_with_parity(out, i, byte, parity_bit);
    }
}

static uint8_t lfsr_rollback_bit(Crypto1* crypto1, uint32_t in, int fb) {
    int out;
    uint8_t ret;
    uint32_t t;

    crypto1->odd &= 0xffffff;
    t = crypto1->odd;
    crypto1->odd = crypto1->even;
    crypto1->even = t;

    out = crypto1->even & 1;
    out ^= LF_POLY_EVEN & (crypto1->even >>= 1);
    out ^= LF_POLY_ODD & crypto1->odd;
    out ^= !!in;
    out ^= (ret = crypto1_filter(crypto1->odd)) & (!!fb);

    crypto1->even |= (nfc_util_even_parity32(out)) << 23;
    return ret;
}

uint32_t crypto1_lfsr_rollback_word(Crypto1* crypto1, uint32_t in, int fb) {
    uint32_t ret = 0;
    for(int i = 31; i >= 0; i--) {
        ret |= lfsr_rollback_bit(crypto1, BEBIT(in, i), fb) << (24 ^ i);
    }
    return ret;
}

bool crypto1_nonce_matches_encrypted_parity_bits(uint32_t nt, uint32_t ks, uint8_t nt_par_enc) {
    return (nfc_util_even_parity8((nt >> 24) & 0xFF) ==
            (((nt_par_enc >> 3) & 1) ^ FURI_BIT(ks, 16))) &&
           (nfc_util_even_parity8((nt >> 16) & 0xFF) ==
            (((nt_par_enc >> 2) & 1) ^ FURI_BIT(ks, 8))) &&
           (nfc_util_even_parity8((nt >> 8) & 0xFF) ==
            (((nt_par_enc >> 1) & 1) ^ FURI_BIT(ks, 0)));
}

bool crypto1_is_weak_prng_nonce(uint32_t nonce) {
    if(nonce == 0) return false;
    uint16_t x = nonce >> 16;
    x = (x & 0xff) << 8 | x >> 8;
    for(uint8_t i = 0; i < 16; i++) {
        x = x >> 1 | (x ^ x >> 2 ^ x >> 3 ^ x >> 5) << 15;
    }
    x = (x & 0xff) << 8 | x >> 8;
    return x == (nonce & 0xFFFF);
}

uint32_t crypto1_decrypt_nt_enc(uint32_t cuid, uint32_t nt_enc, MfClassicKey known_key) {
    uint64_t known_key_int = bit_lib_bytes_to_num_be(known_key.data, 6);
    Crypto1 crypto_temp;
    crypto1_init(&crypto_temp, known_key_int);
    crypto1_word(&crypto_temp, nt_enc ^ cuid, 1);
    uint32_t decrypted_nt_enc =
        (nt_enc ^ crypto1_lfsr_rollback_word(&crypto_temp, nt_enc ^ cuid, 1));
    return decrypted_nt_enc;
}
