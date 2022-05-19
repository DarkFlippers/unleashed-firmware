#include "crypto1.h"
#include "nfc_util.h"
#include <furi.h>

// Algorithm from https://github.com/RfidResearchGroup/proxmark3.git

#define SWAPENDIAN(x) (x = (x >> 8 & 0xff00ff) | (x & 0xff00ff) << 8, x = x >> 16 | x << 16)
#define LF_POLY_ODD (0x29CE5C)
#define LF_POLY_EVEN (0x870804)

#define BEBIT(x, n) FURI_BIT(x, (n) ^ 24)

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

uint8_t crypto1_word(Crypto1* crypto1, uint32_t in, int is_encrypted) {
    furi_assert(crypto1);
    uint32_t out = 0;
    for(uint8_t i = 0; i < 32; i++) {
        out |= crypto1_bit(crypto1, BEBIT(in, i), is_encrypted) << (24 ^ i);
    }
    return out;
}

uint32_t prng_successor(uint32_t x, uint32_t n) {
    SWAPENDIAN(x);
    while(n--) x = x >> 1 | (x >> 16 ^ x >> 18 ^ x >> 19 ^ x >> 21) << 31;

    return SWAPENDIAN(x);
}
