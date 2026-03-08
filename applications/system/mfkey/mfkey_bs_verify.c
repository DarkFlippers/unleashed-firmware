// 32-way SWAR (SIMD Within A Register) verification of candidate LFSR states.
// Each bit position in a uint32_t represents one of 32 parallel lanes.

#pragma GCC optimize("O3")

#include "mfkey_bs_verify.h"
#include "crypto1.h"
#include <string.h>

// VFP register parking: use the M4F's 32 FPU registers (s0-s31) to stash
// slow-changing values during filter execution, freeing GP registers.

#if defined(__arm__) && defined(__ARM_FP)
#define VFP_PARK(var, slot) \
    float slot;             \
    __asm__ volatile("vmov %0, %1" : "=t"(slot) : "r"(var))
#define VFP_UNPARK(var, slot) __asm__ volatile("vmov %0, %1" : "=r"(var) : "t"(slot))
#else
#define VFP_PARK(var, slot)   uint32_t slot = (var)
#define VFP_UNPARK(var, slot) (var) = (slot)
#endif

// Bit-sliced filter function (minimized sum-of-products form)

static inline __attribute__((always_inline)) uint32_t
    crypto1_lut_a(uint32_t d, uint32_t c, uint32_t b, uint32_t a) {
    return (c & d) | (a & c & ~b) | (a & d & ~b) | (b & ~c & ~d);
}

static inline __attribute__((always_inline)) uint32_t
    crypto1_lut_b(uint32_t d, uint32_t c, uint32_t b, uint32_t a) {
    return (b & c & d) | (a & b & ~c) | (c & ~b & ~d) | (d & ~a & ~b);
}

static inline __attribute__((always_inline)) uint32_t
    crypto1_bs_filter(const uint32_t* odd, uint32_t head) {
    const uint32_t* p = odd + head;
    uint32_t f4 = crypto1_lut_a(p[3], p[2], p[1], p[0]);
    uint32_t f3 = crypto1_lut_b(p[7], p[6], p[5], p[4]);
    uint32_t f2 = crypto1_lut_a(p[11], p[10], p[9], p[8]);
    uint32_t f1 = crypto1_lut_a(p[15], p[14], p[13], p[12]);
    uint32_t f0 = crypto1_lut_b(p[19], p[18], p[17], p[16]);

    uint32_t f32 = f3 & f2;
    uint32_t res = (f32 & (f0 | f1));
    res |= (f4 & ((f1 & f3) | ~(f0 | f3)));
    res |= (f0 & ~f2 & ((f1 & ~f4) | ~(f1 | f3)));
    return res;
}

// LFSR polynomial tap XOR functions

static inline __attribute__((always_inline)) uint32_t
    crypto1_bs_xor_taps_odd(const uint32_t* reg, uint32_t head) {
    const uint32_t* p = reg + head;
    uint32_t acc0 = p[2] ^ p[3] ^ p[4];
    uint32_t acc1 = p[6] ^ p[9] ^ p[10];
    uint32_t acc2 = p[11] ^ p[14] ^ p[15];
    uint32_t acc3 = p[16] ^ p[19] ^ p[21];
    return acc0 ^ acc1 ^ acc2 ^ acc3;
}

static inline __attribute__((always_inline)) uint32_t
    crypto1_bs_xor_taps_even(const uint32_t* reg, uint32_t head) {
    const uint32_t* p = reg + head;
    uint32_t acc0 = p[2] ^ p[11] ^ p[16];
    uint32_t acc1 = p[17] ^ p[18] ^ p[23];
    return acc0 ^ acc1;
}

static inline __attribute__((always_inline)) uint32_t
    poly_even_rollback_xor(const uint32_t* even, uint32_t head) {
    const uint32_t* p = even + head;
    return p[2] ^ p[11] ^ p[16] ^ p[17] ^ p[18];
}

// 32x32 SWAR butterfly transpose

static void transpose_32x32(uint32_t* d) {
    uint32_t t, r0, r1, r2, r3, r4, r5, r6, r7;

    // Step 1: 16x16 blocks
    for(int i = 0; i < 16; i++) {
        t = (d[i] >> 16 ^ d[i + 16]) & 0x0000FFFF;
        d[i] ^= t << 16;
        d[i + 16] ^= t;
    }

    // Step 2: 8x8 blocks
    for(int i = 0; i < 32; i += 16) {
        for(int j = 0; j < 8; j++) {
            t = (d[i + j] >> 8 ^ d[i + j + 8]) & 0x00FF00FF;
            d[i + j] ^= t << 8;
            d[i + j + 8] ^= t;
        }
    }

    // Steps 3-5: Process in 8-row blocks entirely in registers
    for(int b = 0; b < 32; b += 8) {
        r0 = d[b + 0];
        r1 = d[b + 1];
        r2 = d[b + 2];
        r3 = d[b + 3];
        r4 = d[b + 4];
        r5 = d[b + 5];
        r6 = d[b + 6];
        r7 = d[b + 7];

        // Step 3: 4x4 blocks
        t = (r0 >> 4 ^ r4) & 0x0F0F0F0F;
        r0 ^= t << 4;
        r4 ^= t;
        t = (r1 >> 4 ^ r5) & 0x0F0F0F0F;
        r1 ^= t << 4;
        r5 ^= t;
        t = (r2 >> 4 ^ r6) & 0x0F0F0F0F;
        r2 ^= t << 4;
        r6 ^= t;
        t = (r3 >> 4 ^ r7) & 0x0F0F0F0F;
        r3 ^= t << 4;
        r7 ^= t;

        // Step 4: 2x2 blocks
        t = (r0 >> 2 ^ r2) & 0x33333333;
        r0 ^= t << 2;
        r2 ^= t;
        t = (r1 >> 2 ^ r3) & 0x33333333;
        r1 ^= t << 2;
        r3 ^= t;
        t = (r4 >> 2 ^ r6) & 0x33333333;
        r4 ^= t << 2;
        r6 ^= t;
        t = (r5 >> 2 ^ r7) & 0x33333333;
        r5 ^= t << 2;
        r7 ^= t;

        // Step 5: 1x1 blocks
        t = (r0 >> 1 ^ r1) & 0x55555555;
        r0 ^= t << 1;
        r1 ^= t;
        t = (r2 >> 1 ^ r3) & 0x55555555;
        r2 ^= t << 1;
        r3 ^= t;
        t = (r4 >> 1 ^ r5) & 0x55555555;
        r4 ^= t << 1;
        r5 ^= t;
        t = (r6 >> 1 ^ r7) & 0x55555555;
        r6 ^= t << 1;
        r7 ^= t;

        d[b + 0] = r0;
        d[b + 1] = r1;
        d[b + 2] = r2;
        d[b + 3] = r3;
        d[b + 4] = r4;
        d[b + 5] = r5;
        d[b + 6] = r6;
        d[b + 7] = r7;
    }
}

void bs_init_from_candidates(Crypto1BitSlice* bs, const BsCandidateBatch* batch) {
    bs->odd_head = 0;
    bs->even_head = 0;

    uint32_t temp_odd[32];
    uint32_t temp_even[32];

    int count = batch->count;
    if(count >= 32) {
        memcpy(temp_odd, batch->odd, 32 * sizeof(uint32_t));
        memcpy(temp_even, batch->even, 32 * sizeof(uint32_t));
    } else {
        memcpy(temp_odd, batch->odd, count * sizeof(uint32_t));
        memcpy(temp_even, batch->even, count * sizeof(uint32_t));
        memset(temp_odd + count, 0, (32 - count) * sizeof(uint32_t));
        memset(temp_even + count, 0, (32 - count) * sizeof(uint32_t));
    }

    transpose_32x32(temp_odd);
    transpose_32x32(temp_even);

    for(int i = 0; i < 24; i++) {
        bs->odd[i] = temp_odd[i];
        bs->odd[i + 24] = temp_odd[i];
        bs->even[i] = temp_even[i];
        bs->even[i + 24] = temp_even[i];
    }
}

// Rollback without keystream collection
static inline __attribute__((always_inline)) void
    bs_rollback_word_noret(Crypto1BitSlice* bs, uint32_t in, uint32_t fb_mask) {
    uint32_t* odd_ptr = bs->odd;
    uint32_t* even_ptr = bs->even;
    uint32_t oh = bs->odd_head;
    uint32_t eh = bs->even_head;

    // Process first 16 bits (i = 31..16)
    for(int i = 31; i >= 16; i--) {
        uint32_t* tmp_ptr = odd_ptr;
        odd_ptr = even_ptr;
        even_ptr = tmp_ptr;

        uint32_t tmp_head = oh;
        oh = eh;
        eh = tmp_head;

        int bit_pos = 24 ^ i; // Crypto1 big-endian bit ordering
        uint32_t in_bits = ((in >> bit_pos) & 1) ? 0xFFFFFFFF : 0; // Broadcast bit to all 32 lanes

        uint32_t extracted = even_ptr[eh];
        VFP_PARK(extracted, _vfp_extracted);

        uint32_t ks = crypto1_bs_filter(odd_ptr, oh);
        VFP_UNPARK(extracted, _vfp_extracted);
        uint32_t new_eh = eh + 1;

        uint32_t recovered_msb = extracted;
        recovered_msb ^= poly_even_rollback_xor(even_ptr, new_eh);
        recovered_msb ^= crypto1_bs_xor_taps_odd(odd_ptr, oh);
        recovered_msb ^= in_bits;
        recovered_msb ^= (ks & fb_mask);

        even_ptr[new_eh + 23] = recovered_msb;
        if(new_eh + 23 >= 24) even_ptr[new_eh + 23 - 24] = recovered_msb;

        eh = new_eh;
    }

    // Intermediate normalization
    if(oh >= 24) oh -= 24;
    if(eh >= 24) eh -= 24;

    // Process remaining 16 bits (i = 15..0)
    for(int i = 15; i >= 0; i--) {
        uint32_t* tmp_ptr = odd_ptr;
        odd_ptr = even_ptr;
        even_ptr = tmp_ptr;

        uint32_t tmp_head = oh;
        oh = eh;
        eh = tmp_head;

        int bit_pos = 24 ^ i;
        uint32_t in_bits = ((in >> bit_pos) & 1) ? 0xFFFFFFFF : 0; // Sign-extend bit without UB

        uint32_t extracted = even_ptr[eh];
        VFP_PARK(extracted, _vfp_extracted);

        uint32_t ks = crypto1_bs_filter(odd_ptr, oh);
        VFP_UNPARK(extracted, _vfp_extracted);
        uint32_t new_eh = eh + 1;

        uint32_t recovered_msb = extracted;
        recovered_msb ^= poly_even_rollback_xor(even_ptr, new_eh);
        recovered_msb ^= crypto1_bs_xor_taps_odd(odd_ptr, oh);
        recovered_msb ^= in_bits;
        recovered_msb ^= (ks & fb_mask);

        even_ptr[new_eh + 23] = recovered_msb;
        if(new_eh + 23 >= 24) even_ptr[new_eh + 23 - 24] = recovered_msb;

        eh = new_eh;
    }

    if(oh >= 24) oh -= 24;
    if(eh >= 24) eh -= 24;

    bs->odd_head = oh;
    bs->even_head = eh;
}

static inline __attribute__((always_inline)) void
    bs_crypt_word_noret(Crypto1BitSlice* bs, uint32_t in, uint32_t enc_mask) {
    uint32_t* odd_ptr = bs->odd;
    uint32_t* even_ptr = bs->even;
    uint32_t oh = bs->odd_head;
    uint32_t eh = bs->even_head;

    for(int i = 0; i < 32; i++) {
        int bit_pos = 24 ^ i;
        uint32_t in_bits = ((in >> bit_pos) & 1) ? 0xFFFFFFFF : 0; // Sign-extend bit without UB

        VFP_PARK(in_bits, _vfp_in_bits);
        uint32_t ks = crypto1_bs_filter(odd_ptr, oh);
        VFP_UNPARK(in_bits, _vfp_in_bits);
        uint32_t feed = (ks & enc_mask) ^ in_bits;
        feed ^= crypto1_bs_xor_taps_odd(odd_ptr, oh);
        feed ^= crypto1_bs_xor_taps_even(even_ptr, eh);

        uint32_t new_eh = (eh == 0) ? 23 : (eh - 1);

        even_ptr[new_eh] = feed;
        even_ptr[new_eh + 24] = feed;
        eh = new_eh;

        uint32_t* tmp_ptr = odd_ptr;
        odd_ptr = even_ptr;
        even_ptr = tmp_ptr;

        uint32_t tmp_head = oh;
        oh = eh;
        eh = tmp_head;
    }

    bs->odd_head = oh;
    bs->even_head = eh;
}

// Fused rollback + keystream comparison with byte-boundary early exit.
// fb_mask: 0 when keystream does not feed back into LFSR.

static inline __attribute__((always_inline)) uint32_t bs_rollback_word_check_ks(
    Crypto1BitSlice* bs,
    uint32_t in,
    uint32_t fb_mask,
    uint32_t expected,
    uint32_t alive) {
    uint32_t* odd_ptr = bs->odd;
    uint32_t* even_ptr = bs->even;
    uint32_t oh = bs->odd_head;
    uint32_t eh = bs->even_head;

    for(int i = 31; i >= 0; i--) {
        uint32_t* tmp_ptr = odd_ptr;
        odd_ptr = even_ptr;
        even_ptr = tmp_ptr;

        uint32_t tmp_head = oh;
        oh = eh;
        eh = tmp_head;

        int bit_pos = 24 ^ i;
        uint32_t in_bits = ((in >> bit_pos) & 1) ? 0xFFFFFFFF : 0;

        uint32_t extracted = even_ptr[eh];
        uint32_t new_eh = eh + 1;

        // Park values not needed during filter in VFP registers
        VFP_PARK(alive, _vfp_alive);
        VFP_PARK(extracted, _vfp_extracted);
        VFP_PARK(new_eh, _vfp_new_eh);

        uint32_t ks = crypto1_bs_filter(odd_ptr, oh);

        // Restore from VFP
        VFP_UNPARK(alive, _vfp_alive);
        VFP_UNPARK(extracted, _vfp_extracted);
        VFP_UNPARK(new_eh, _vfp_new_eh);

        // Compare keystream bit against expected
        uint32_t exp_broadcast = ((expected >> bit_pos) & 1) ? 0xFFFFFFFF : 0;
        alive &= ~(ks ^ exp_broadcast);

        // Rollback LFSR step
        uint32_t recovered_msb = extracted;
        recovered_msb ^= poly_even_rollback_xor(even_ptr, new_eh);
        recovered_msb ^= crypto1_bs_xor_taps_odd(odd_ptr, oh);
        recovered_msb ^= in_bits;
        recovered_msb ^= (ks & fb_mask);

        even_ptr[new_eh + 23] = recovered_msb;
        if(new_eh + 23 >= 24) even_ptr[new_eh + 23 - 24] = recovered_msb;

        eh = new_eh;

        // Early exit at byte boundaries
        if((i & 7) == 0 && !alive) {
            if(oh >= 24) oh -= 24;
            if(eh >= 24) eh -= 24;
            bs->odd_head = oh;
            bs->even_head = eh;
            return 0;
        }
    }

    if(oh >= 24) oh -= 24;
    if(eh >= 24) eh -= 24;
    bs->odd_head = oh;
    bs->even_head = eh;
    return alive;
}

// Fused forward crypt + keystream comparison with byte-boundary early exit.
// enc_mask: 0 when keystream does not feed back into LFSR.

static inline __attribute__((always_inline)) uint32_t bs_crypt_word_check_ks(
    Crypto1BitSlice* bs,
    uint32_t in,
    uint32_t enc_mask,
    uint32_t expected,
    uint32_t alive) {
    uint32_t* odd_ptr = bs->odd;
    uint32_t* even_ptr = bs->even;
    uint32_t oh = bs->odd_head;
    uint32_t eh = bs->even_head;

    for(int i = 0; i < 32; i++) {
        int bit_pos = 24 ^ i;
        uint32_t in_bits = ((in >> bit_pos) & 1) ? 0xFFFFFFFF : 0;

        // Park alive in VFP during heavy filter computation
        VFP_PARK(alive, _vfp_alive);

        uint32_t ks = crypto1_bs_filter(odd_ptr, oh);

        // Restore alive from VFP
        VFP_UNPARK(alive, _vfp_alive);

        // Compare keystream bit against expected
        uint32_t exp_broadcast = ((expected >> bit_pos) & 1) ? 0xFFFFFFFF : 0;
        alive &= ~(ks ^ exp_broadcast);

        // LFSR advance
        uint32_t feed = (ks & enc_mask) ^ in_bits;
        feed ^= crypto1_bs_xor_taps_odd(odd_ptr, oh);
        feed ^= crypto1_bs_xor_taps_even(even_ptr, eh);

        uint32_t new_eh = (eh == 0) ? 23 : (eh - 1);
        even_ptr[new_eh] = feed;
        even_ptr[new_eh + 24] = feed;
        eh = new_eh;

        uint32_t* tmp_ptr = odd_ptr;
        odd_ptr = even_ptr;
        even_ptr = tmp_ptr;
        uint32_t tmp_head = oh;
        oh = eh;
        eh = tmp_head;

        // Early exit at byte boundaries
        if((i & 7) == 7 && !alive) {
            bs->odd_head = oh;
            bs->even_head = eh;
            return 0;
        }
    }

    bs->odd_head = oh;
    bs->even_head = eh;
    return alive;
}

// mfkey32 verification kernel
uint32_t bs_verify_batch_32(Crypto1BitSlice* bs, MfClassicNonce* nonce, uint32_t alive) {
    // Checkpoint 1: rollback with keystream check
    uint32_t expected1 = nonce->ar0_enc ^ nonce->p64;
    alive = bs_rollback_word_check_ks(bs, 0, 0, expected1, alive);
    if(!alive) return 0;

    // Intermediate rollback/crypt (no keystream check)
    bs_rollback_word_noret(bs, nonce->nr0_enc, 0xFFFFFFFF);
    bs_rollback_word_noret(bs, nonce->uid_xor_nt0, 0);
    bs_crypt_word_noret(bs, nonce->uid_xor_nt1, 0);
    bs_crypt_word_noret(bs, nonce->nr1_enc, 0xFFFFFFFF);

    // Checkpoint 2: forward crypt with keystream check
    uint32_t expected2 = nonce->ar1_enc ^ nonce->p64b;
    alive = bs_crypt_word_check_ks(bs, 0, 0, expected2, alive);

    return alive;
}

void bs_extract_key(const BsCandidateBatch* batch, int lane, MfClassicNonce* nonce) {
    struct Crypto1State t;
    t.odd = batch->odd[lane];
    t.even = batch->even[lane];

    if(nonce->attack == mfkey32) {
        napi_lfsr_rollback_word(&t, 0, 0);
        rollback_word_noret(&t, nonce->nr0_enc, 1);
        rollback_word_noret(&t, nonce->uid_xor_nt0, 0);
    } else if(nonce->attack == static_nested) {
        rollback_word_noret(&t, nonce->uid_xor_nt1, 0);
    } else {
        napi_lfsr_rollback_word(&t, nonce->uid_xor_nt0, 0);
    }

    crypto1_get_lfsr(&t, &nonce->key);
}

// static_nested verification kernel
uint32_t bs_verify_batch_32_nested(Crypto1BitSlice* bs, MfClassicNonce* nonce, uint32_t alive) {
    // Step 1: Rollback uid_xor_nt1 (fb=0) — no keystream check
    bs_rollback_word_noret(bs, nonce->uid_xor_nt1, 0);

    // Step 2: Forward crypt uid_xor_nt0 (enc_mask=0) with keystream check
    alive = bs_crypt_word_check_ks(bs, nonce->uid_xor_nt0, 0, nonce->ks1_1_enc, alive);

    return alive;
}

// Scalar parity validation (cold path, typically 0-2 survivors)
static uint32_t validate_survivors_parity(
    const BsCandidateBatch* batch,
    MfClassicNonce* nonce,
    uint32_t alive) {
    uint32_t parity_valid = 0;
    uint32_t remaining = alive;
    while(remaining) {
        int lane = __builtin_ctz(remaining);
        remaining &= remaining - 1;

        struct Crypto1State t;
        t.odd = batch->odd[lane];
        t.even = batch->even[lane];
        napi_lfsr_rollback_word(&t, nonce->uid_xor_nt0, 0);

        uint8_t pk;
        struct Crypto1State temp = {t.odd, t.even};
        if((crypt_word_par(&temp, nonce->uid_xor_nt0, 0, nonce->nt0, &pk) == nonce->ks1_1_enc) &&
           (pk == nonce->par_1)) {
            parity_valid |= (1U << lane);
        }
    }
    return parity_valid;
}

// static_encrypted verification kernel
// Hybrid: bitsliced keystream pruning + scalar parity on survivors.
uint32_t bs_verify_batch_32_encrypted(
    Crypto1BitSlice* bs,
    const BsCandidateBatch* batch,
    MfClassicNonce* nonce,
    uint32_t alive) {
    alive = bs_rollback_word_check_ks(bs, nonce->uid_xor_nt0, 0, nonce->ks1_1_enc, alive);
    if(!alive) return 0;
    return validate_survivors_parity(batch, nonce, alive);
}
