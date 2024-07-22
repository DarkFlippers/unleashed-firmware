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

uint32_t prng_successor(uint32_t x, uint32_t n) {
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

    nt_num = prng_successor(nt_num, 32);
    for(size_t i = 4; i < 8; i++) {
        nt_num = prng_successor(nt_num, 8);
        uint8_t byte = crypto1_byte(crypto, 0, 0) ^ (uint8_t)(nt_num);
        bool parity_bit = ((crypto1_filter(crypto->odd) ^ nfc_util_odd_parity8(nt_num)) & 0x01);
        bit_buffer_set_byte_with_parity(out, i, byte, parity_bit);
    }
}
