#ifndef CRYPTO1_H
#define CRYPTO1_H

#include <inttypes.h>
#include "mfkey.h"
#include <nfc/helpers/nfc_util.h>
#include <nfc/protocols/mf_classic/mf_classic.h>

#define LF_POLY_ODD  (0x29CE5C)
#define LF_POLY_EVEN (0x870804)
#define BIT(x, n)    ((x) >> (n) & 1)
#define BEBIT(x, n)  BIT(x, (n) ^ 24)
#define SWAPENDIAN(x) \
    ((x) = ((x) >> 8 & 0xff00ff) | ((x) & 0xff00ff) << 8, (x) = (x) >> 16 | (x) << 16)

static inline uint32_t prng_successor(uint32_t x, uint32_t n);
static inline int filter(uint32_t const x);
static inline uint8_t evenparity32(uint32_t x);
static inline void update_contribution(unsigned int data[], int item, int mask1, int mask2);
void crypto1_get_lfsr(struct Crypto1State* state, MfClassicKey* lfsr);
static inline uint32_t crypt_word(struct Crypto1State* s);
static inline void crypt_word_noret(struct Crypto1State* s, uint32_t in, int x);
static inline uint32_t crypt_word_ret(struct Crypto1State* s, uint32_t in, int x);
static uint32_t crypt_word_par(
    struct Crypto1State* s,
    uint32_t in,
    int is_encrypted,
    uint32_t nt_plain,
    uint8_t* parity_keystream_bits);
static inline void rollback_word_noret(struct Crypto1State* s, uint32_t in, int x);
static inline uint8_t napi_lfsr_rollback_bit(struct Crypto1State* s, uint32_t in, int fb);
static inline uint32_t napi_lfsr_rollback_word(struct Crypto1State* s, uint32_t in, int fb);

static const uint8_t lookup1[256] = {
    0, 0,  16, 16, 0,  16, 0,  0,  0, 16, 0,  0,  16, 16, 16, 16, 0, 0,  16, 16, 0,  16, 0,  0,
    0, 16, 0,  0,  16, 16, 16, 16, 0, 0,  16, 16, 0,  16, 0,  0,  0, 16, 0,  0,  16, 16, 16, 16,
    8, 8,  24, 24, 8,  24, 8,  8,  8, 24, 8,  8,  24, 24, 24, 24, 8, 8,  24, 24, 8,  24, 8,  8,
    8, 24, 8,  8,  24, 24, 24, 24, 8, 8,  24, 24, 8,  24, 8,  8,  8, 24, 8,  8,  24, 24, 24, 24,
    0, 0,  16, 16, 0,  16, 0,  0,  0, 16, 0,  0,  16, 16, 16, 16, 0, 0,  16, 16, 0,  16, 0,  0,
    0, 16, 0,  0,  16, 16, 16, 16, 8, 8,  24, 24, 8,  24, 8,  8,  8, 24, 8,  8,  24, 24, 24, 24,
    0, 0,  16, 16, 0,  16, 0,  0,  0, 16, 0,  0,  16, 16, 16, 16, 0, 0,  16, 16, 0,  16, 0,  0,
    0, 16, 0,  0,  16, 16, 16, 16, 8, 8,  24, 24, 8,  24, 8,  8,  8, 24, 8,  8,  24, 24, 24, 24,
    8, 8,  24, 24, 8,  24, 8,  8,  8, 24, 8,  8,  24, 24, 24, 24, 0, 0,  16, 16, 0,  16, 0,  0,
    0, 16, 0,  0,  16, 16, 16, 16, 8, 8,  24, 24, 8,  24, 8,  8,  8, 24, 8,  8,  24, 24, 24, 24,
    8, 8,  24, 24, 8,  24, 8,  8,  8, 24, 8,  8,  24, 24, 24, 24};
static const uint8_t lookup2[256] = {
    0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4,
    4, 4, 4, 2, 2, 6, 6, 2, 6, 2, 2, 2, 6, 2, 2, 6, 6, 6, 6, 2, 2, 6, 6, 2, 6, 2, 2, 2, 6,
    2, 2, 6, 6, 6, 6, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 2, 2, 6, 6, 2, 6, 2,
    2, 2, 6, 2, 2, 6, 6, 6, 6, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4,
    0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 2,
    2, 6, 6, 2, 6, 2, 2, 2, 6, 2, 2, 6, 6, 6, 6, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4,
    4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 2, 2, 6, 6, 2, 6, 2, 2, 2, 6, 2,
    2, 6, 6, 6, 6, 2, 2, 6, 6, 2, 6, 2, 2, 2, 6, 2, 2, 6, 6, 6, 6, 2, 2, 6, 6, 2, 6, 2, 2,
    2, 6, 2, 2, 6, 6, 6, 6, 2, 2, 6, 6, 2, 6, 2, 2, 2, 6, 2, 2, 6, 6, 6, 6};

static inline int filter(uint32_t const x) {
    uint32_t f;
    f = lookup1[x & 0xff] | lookup2[(x >> 8) & 0xff];
    f |= 0x0d938 >> (x >> 16 & 0xf) & 1;
    return BIT(0xEC57E80A, f);
}

#ifndef __ARM_ARCH_7EM__
static inline uint8_t evenparity32(uint32_t x) {
    return __builtin_parity(x);
}
#endif

#ifdef __ARM_ARCH_7EM__
static inline uint8_t evenparity32(uint32_t x) {
    uint32_t result;
    __asm__ volatile("eor r1, %[x], %[x], lsr #16  \n\t" // r1 = x ^ (x >> 16)
                     "eor r1, r1, r1, lsr #8       \n\t" // r1 = r1 ^ (r1 >> 8)
                     "eor r1, r1, r1, lsr #4       \n\t" // r1 = r1 ^ (r1 >> 4)
                     "eor r1, r1, r1, lsr #2       \n\t" // r1 = r1 ^ (r1 >> 2)
                     "eor r1, r1, r1, lsr #1       \n\t" // r1 = r1 ^ (r1 >> 1)
                     "and %[result], r1, #1        \n\t" // result = r1 & 1
                     : [result] "=r"(result)
                     : [x] "r"(x)
                     : "r1");
    return result;
}
#endif

static inline void update_contribution(unsigned int data[], int item, int mask1, int mask2) {
    int p = data[item] >> 25;
    p = p << 1 | evenparity32(data[item] & mask1);
    p = p << 1 | evenparity32(data[item] & mask2);
    data[item] = p << 24 | (data[item] & 0xffffff);
}

static inline uint32_t crypt_word(struct Crypto1State* s) {
    // "in" and "x" are always 0 (last iteration)
    uint32_t res_ret = 0;
    uint32_t feedin, t;
    for(int i = 0; i <= 31; i++) {
        res_ret |= (filter(s->odd) << (24 ^ i)); //-V629
        feedin = LF_POLY_EVEN & s->even;
        feedin ^= LF_POLY_ODD & s->odd;
        s->even = s->even << 1 | (evenparity32(feedin));
        t = s->odd, s->odd = s->even, s->even = t;
    }
    return res_ret;
}

static inline void crypt_word_noret(struct Crypto1State* s, uint32_t in, int x) {
    uint8_t ret;
    uint32_t feedin, t, next_in;
    for(int i = 0; i <= 31; i++) {
        next_in = BEBIT(in, i);
        ret = filter(s->odd);
        feedin = ret & (!!x);
        feedin ^= LF_POLY_EVEN & s->even;
        feedin ^= LF_POLY_ODD & s->odd;
        feedin ^= !!next_in;
        s->even = s->even << 1 | (evenparity32(feedin));
        t = s->odd, s->odd = s->even, s->even = t;
    }
    return;
}

static inline uint32_t crypt_word_ret(struct Crypto1State* s, uint32_t in, int x) {
    uint32_t ret = 0;
    uint32_t feedin, t, next_in;
    uint8_t next_ret;
    for(int i = 0; i <= 31; i++) {
        next_in = BEBIT(in, i);
        next_ret = filter(s->odd);
        feedin = next_ret & (!!x);
        feedin ^= LF_POLY_EVEN & s->even;
        feedin ^= LF_POLY_ODD & s->odd;
        feedin ^= !!next_in;
        s->even = s->even << 1 | (evenparity32(feedin));
        t = s->odd, s->odd = s->even, s->even = t;
        ret |= next_ret << (24 ^ i);
    }
    return ret;
}

static uint8_t get_nth_byte(uint32_t value, int n) {
    if(n < 0 || n > 3) {
        // Handle invalid input
        return 0;
    }
    return (value >> (8 * (3 - n))) & 0xFF;
}

static uint8_t crypt_bit(struct Crypto1State* s, uint8_t in, int is_encrypted) {
    uint32_t feedin, t;
    uint8_t ret = filter(s->odd);
    feedin = ret & !!is_encrypted;
    feedin ^= !!in;
    feedin ^= LF_POLY_ODD & s->odd;
    feedin ^= LF_POLY_EVEN & s->even;
    s->even = s->even << 1 | evenparity32(feedin);
    t = s->odd, s->odd = s->even, s->even = t;
    return ret;
}

static inline uint32_t crypt_word_par(
    struct Crypto1State* s,
    uint32_t in,
    int is_encrypted,
    uint32_t nt_plain,
    uint8_t* parity_keystream_bits) {
    uint32_t ret = 0;
    *parity_keystream_bits = 0; // Reset parity keystream bits

    for(int i = 0; i < 32; i++) {
        uint8_t bit = crypt_bit(s, BEBIT(in, i), is_encrypted);
        ret |= bit << (24 ^ i);
        // Save keystream parity bit
        if((i + 1) % 8 == 0) {
            *parity_keystream_bits |=
                (filter(s->odd) ^ nfc_util_even_parity8(get_nth_byte(nt_plain, i / 8)))
                << (3 - (i / 8));
        }
    }
    return ret;
}

static inline void rollback_word_noret(struct Crypto1State* s, uint32_t in, int x) {
    uint8_t ret;
    uint32_t feedin, t, next_in;
    for(int i = 31; i >= 0; i--) {
        next_in = BEBIT(in, i);
        s->odd &= 0xffffff;
        t = s->odd, s->odd = s->even, s->even = t;
        ret = filter(s->odd);
        feedin = ret & (!!x);
        feedin ^= s->even & 1;
        feedin ^= LF_POLY_EVEN & (s->even >>= 1);
        feedin ^= LF_POLY_ODD & s->odd;
        feedin ^= !!next_in;
        s->even |= (evenparity32(feedin)) << 23;
    }
    return;
}

// TODO:
/*
uint32_t rollback_word(struct Crypto1State *s, uint32_t in, int x) {
    uint32_t res_ret = 0;
    uint8_t ret;
    uint32_t feedin, t, next_in;
    for (int i = 31; i >= 0; i--) {
        next_in = BEBIT(in, i);
        s->odd &= 0xffffff;
        t = s->odd, s->odd = s->even, s->even = t;
        ret = filter(s->odd);
        feedin = ret & (!!x);
        feedin ^= s->even & 1;
        feedin ^= LF_POLY_EVEN & (s->even >>= 1);
        feedin ^= LF_POLY_ODD & s->odd;
        feedin ^= !!next_in;
        s->even |= (evenparity32(feedin)) << 23;
        res_ret |= (ret << (24 ^ i));
    }
    return res_ret;
}
*/

uint8_t napi_lfsr_rollback_bit(struct Crypto1State* s, uint32_t in, int fb) {
    int out;
    uint8_t ret;
    uint32_t t;
    s->odd &= 0xffffff;
    t = s->odd, s->odd = s->even, s->even = t;

    out = s->even & 1;
    out ^= LF_POLY_EVEN & (s->even >>= 1);
    out ^= LF_POLY_ODD & s->odd;
    out ^= !!in;
    out ^= (ret = filter(s->odd)) & !!fb;

    s->even |= evenparity32(out) << 23;
    return ret;
}

uint32_t napi_lfsr_rollback_word(struct Crypto1State* s, uint32_t in, int fb) {
    int i;
    uint32_t ret = 0;
    for(i = 31; i >= 0; --i)
        ret |= napi_lfsr_rollback_bit(s, BEBIT(in, i), fb) << (i ^ 24);
    return ret;
}

static inline uint32_t prng_successor(uint32_t x, uint32_t n) {
    SWAPENDIAN(x);
    while(n--)
        x = x >> 1 | (x >> 16 ^ x >> 18 ^ x >> 19 ^ x >> 21) << 31;
    return SWAPENDIAN(x);
}

#endif // CRYPTO1_H
