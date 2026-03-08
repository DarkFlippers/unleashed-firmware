#ifndef CRYPTO1_H
#define CRYPTO1_H

#include <inttypes.h>

#include "mfkey.h"
#include <nfc/helpers/nfc_util.h>
#include <nfc/protocols/mf_classic/mf_classic.h>

#define LF_POLY_ODD  (0x29CE5C)
#define LF_POLY_EVEN (0x870804)

// Precomputed mask constants for extend_table
#define CONST_M1_1 (LF_POLY_EVEN << 1 | 1)
#define CONST_M2_1 (LF_POLY_ODD << 1)
#define CONST_M1_2 (LF_POLY_ODD)
#define CONST_M2_2 (LF_POLY_EVEN << 1 | 1)

#define BIT(x, n)   ((x) >> (n) & 1)
#define BEBIT(x, n) BIT(x, (n) ^ 24)
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

static const uint8_t __attribute__((aligned(4))) lookup1[256] = {
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
static const uint8_t __attribute__((aligned(4))) lookup2[256] = {
    0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4,
    4, 4, 4, 2, 2, 6, 6, 2, 6, 2, 2, 2, 6, 2, 2, 6, 6, 6, 6, 2, 2, 6, 6, 2, 6, 2, 2, 2, 6,
    2, 2, 6, 6, 6, 6, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 2, 2, 6, 6, 2, 6, 2,
    2, 2, 6, 2, 2, 6, 6, 6, 6, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4,
    0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 2,
    2, 6, 6, 2, 6, 2, 2, 2, 6, 2, 2, 6, 6, 6, 6, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4,
    4, 4, 0, 0, 4, 4, 0, 4, 0, 0, 0, 4, 0, 0, 4, 4, 4, 4, 2, 2, 6, 6, 2, 6, 2, 2, 2, 6, 2,
    2, 6, 6, 6, 6, 2, 2, 6, 6, 2, 6, 2, 2, 2, 6, 2, 2, 6, 6, 6, 6, 2, 2, 6, 6, 2, 6, 2, 2,
    2, 6, 2, 2, 6, 6, 6, 6, 2, 2, 6, 6, 2, 6, 2, 2, 2, 6, 2, 2, 6, 6, 6, 6};

static inline __attribute__((always_inline)) int filter(uint32_t const x) {
    uint32_t f;
    f = lookup1[x & 0xff] | lookup2[(x >> 8) & 0xff];
    f |= 0x0d938 >> (x >> 16 & 0xf) & 1;
    return BIT(0xEC57E80A, f);
}

// Optimized: compute filter(v) and filter(v|1) with shared work
// Returns packed (f1 << 1 | f0) to avoid pointer overhead
static inline __attribute__((always_inline)) uint32_t filter_pair(uint32_t v) {
    // Shared indices (v|1 only changes bit 0)
    uint32_t idx_hi = (v >> 8) & 0xff;
    uint32_t idx_nib = (v >> 16) & 0xf;

    // Shared lookups and computation
    uint8_t l2 = lookup2[idx_hi];
    uint32_t nib_bit = (0x0d938 >> idx_nib) & 1;
    uint32_t shared = l2 | nib_bit;

    // lookup1 differs only in bit 0
    uint32_t idx0_lo = v & 0xff;
    uint8_t l1_0 = lookup1[idx0_lo];
    uint8_t l1_1 = lookup1[idx0_lo | 1];

    // Compute both filters
    int f0 = BIT(0xEC57E80A, l1_0 | shared);
    int f1 = BIT(0xEC57E80A, l1_1 | shared);

    // Pack: bit 0 = f0, bit 1 = f1
    return (uint32_t)f0 | ((uint32_t)f1 << 1);
}

// Compute filter pair using pre-XOR'd filter constant
// adj_filter = 0xEC57E80A ^ (-(uint32_t)BIT(xks, round))
// Returns f0/f1 already XOR'd with xks_bit, eliminating xks_bit as a variable
static inline __attribute__((always_inline)) uint32_t
    filter_pair_xor(uint32_t v, uint32_t adj_filter) {
    uint32_t idx_hi = (v >> 8) & 0xff;
    uint32_t idx_nib = (v >> 16) & 0xf;
    uint8_t l2 = lookup2[idx_hi];
    uint32_t nib_bit = (0x0d938 >> idx_nib) & 1;
    uint32_t shared = l2 | nib_bit;
    uint32_t idx0_lo = v & 0xff;
    uint8_t l1_0 = lookup1[idx0_lo];
    uint8_t l1_1 = lookup1[idx0_lo | 1];
    int f0 = BIT(adj_filter, l1_0 | shared);
    int f1 = BIT(adj_filter, l1_1 | shared);
    return (uint32_t)f0 | ((uint32_t)f1 << 1);
}

// Unpack macros for filter_pair result
#define FILTER_F0(fp) ((fp) & 1)
#define FILTER_F1(fp) (((fp) >> 1) & 1)

// Optimized filter_pair with precomputed nibble bit
// nib_bit must be 0 or 1 (the result of (0x0d938 >> nibble) & 1)
static inline __attribute__((always_inline)) uint32_t
    filter_pair_with_nib(uint32_t v, uint32_t nib_bit) {
    uint32_t idx_hi = (v >> 8) & 0xff;

    uint8_t l2 = lookup2[idx_hi];
    uint32_t shared = l2 | nib_bit; // nib_bit precomputed, no shift/mask needed

    uint32_t idx0_lo = v & 0xff;
    uint8_t l1_0 = lookup1[idx0_lo];
    uint8_t l1_1 = lookup1[idx0_lo | 1];

    int f0 = BIT(0xEC57E80A, l1_0 | shared);
    int f1 = BIT(0xEC57E80A, l1_1 | shared);

    return (uint32_t)f0 | ((uint32_t)f1 << 1);
}

// Register-based parity update (avoids array access overhead)
// Calculates new state with parity bits without array/pointer access
static inline __attribute__((always_inline)) uint32_t
    update_contribution_reg(uint32_t v, int mask1, int mask2) {
    uint32_t p = v >> 25;
    p = (p << 1) | evenparity32(v & mask1);
    p = (p << 1) | evenparity32(v & mask2);
    return (p << 24) | (v & 0xffffff);
}

static inline __attribute__((always_inline)) uint8_t evenparity32(uint32_t x) {
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return (uint8_t)(x & 1);
}

static inline __attribute__((always_inline)) void
    update_contribution(unsigned int data[], int item, int mask1, int mask2) {
    unsigned int p = data[item] >> 25; // Use unsigned to avoid UB on left shift
    p = p << 1 | evenparity32(data[item] & mask1);
    p = p << 1 | evenparity32(data[item] & mask2);
    data[item] = (p << 24) | (data[item] & 0xffffff);
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
        ret |= (uint32_t)napi_lfsr_rollback_bit(s, BEBIT(in, i), fb) << (i ^ 24);
    return ret;
}

static inline uint32_t prng_successor(uint32_t x, uint32_t n) {
    SWAPENDIAN(x);
    while(n--)
        x = x >> 1 | (x >> 16 ^ x >> 18 ^ x >> 19 ^ x >> 21) << 31;
    return SWAPENDIAN(x);
}

#endif // CRYPTO1_H
