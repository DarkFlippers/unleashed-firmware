#include "crypto1.h"
#include <string.h>

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

uint32_t crypto1_filter(uint32_t in) {
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
    crypto1->even = crypto1->even << 1 | (evenparity32(feed));

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
    while(n--) x = x >> 1 | (x >> 16 ^ x >> 18 ^ x >> 19 ^ x >> 21) << 31;

    return SWAPENDIAN(x);
}

void crypto1_decrypt(
    Crypto1* crypto,
    uint8_t* encrypted_data,
    uint16_t encrypted_data_bits,
    uint8_t* decrypted_data) {
    furi_assert(crypto);
    furi_assert(encrypted_data);
    furi_assert(decrypted_data);

    if(encrypted_data_bits < 8) {
        uint8_t decrypted_byte = 0;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_data[0], 0)) << 0;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_data[0], 1)) << 1;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_data[0], 2)) << 2;
        decrypted_byte |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(encrypted_data[0], 3)) << 3;
        decrypted_data[0] = decrypted_byte;
    } else {
        for(size_t i = 0; i < encrypted_data_bits / 8; i++) {
            decrypted_data[i] = crypto1_byte(crypto, 0, 0) ^ encrypted_data[i];
        }
    }
}

void crypto1_encrypt(
    Crypto1* crypto,
    uint8_t* keystream,
    uint8_t* plain_data,
    uint16_t plain_data_bits,
    uint8_t* encrypted_data,
    uint8_t* encrypted_parity) {
    furi_assert(crypto);
    furi_assert(plain_data);
    furi_assert(encrypted_data);
    furi_assert(encrypted_parity);

    if(plain_data_bits < 8) {
        encrypted_data[0] = 0;
        for(size_t i = 0; i < plain_data_bits; i++) {
            encrypted_data[0] |= (crypto1_bit(crypto, 0, 0) ^ FURI_BIT(plain_data[0], i)) << i;
        }
    } else {
        memset(encrypted_parity, 0, plain_data_bits / 8 + 1);
        for(uint8_t i = 0; i < plain_data_bits / 8; i++) {
            encrypted_data[i] = crypto1_byte(crypto, keystream ? keystream[i] : 0, 0) ^
                                plain_data[i];
            encrypted_parity[i / 8] |=
                (((crypto1_filter(crypto->odd) ^ oddparity8(plain_data[i])) & 0x01)
                 << (7 - (i & 0x0007)));
        }
    }
}