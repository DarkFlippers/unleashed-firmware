#ifndef MFKEY_DEDUP_H
#define MFKEY_DEDUP_H

#include "mfkey.h"
#include "crypto1.h"
#include <stdint.h>
#include <stdbool.h>

#define OPT_BARRIER(x) __asm__ volatile("" : "+r"(x))

// Golden ratio multiply-shift hash for 20-bit semi-states → 11-bit output
#define FIB_HASH_20BIT(x) (((x) * 2654435769u) >> 21)

// Eliminates candidates before tree expansion by checking rounds 1-3 in advance.
// Returns 1 if any path survives to round 4, else 0.
static inline __attribute__((always_inline)) int
    prefilter_rounds_1_3(uint32_t semi_state, int oks) {
    OPT_BARRIER(oks); // Prevent hoisting of BIT(oks, round) extractions
    // Precompute nibble bits for rounds 1-3
    // Round 1: nibble = (semi_state >> 15) & 0xF
    // Round 2: nibble = (semi_state >> 14) & 0xF
    // Round 3: nibble = (semi_state >> 13) & 0xF
    uint32_t nib1 = ((0x0d938 >> ((semi_state >> 15) & 0xF)) & 1);
    uint32_t nib2 = ((0x0d938 >> ((semi_state >> 14) & 0xF)) & 1);
    uint32_t nib3 = ((0x0d938 >> ((semi_state >> 13) & 0xF)) & 1);

    // Round 1: 2 potential children -> 2-bit mask
    uint32_t v = semi_state << 1;
    int target = BIT(oks, 1);
    uint32_t fp = filter_pair_with_nib(v, nib1);
    int f0 = FILTER_F0(fp);
    int f1 = FILTER_F1(fp);

    // r1_valid: bit 0 = v survives, bit 1 = v|1 survives
    int r1_valid = ((f0 == target) << 0) | ((f1 == target) << 1);
    if(!r1_valid) return 0;

    // Round 2: up to 4 grandchildren -> 4-bit mask
    // Tracks exactly which grandchildren survive
    target = BIT(oks, 2);
    int r2_valid = 0;

    if(r1_valid & 1) { // v survived R1
        fp = filter_pair_with_nib(v << 1, nib2);
        f0 = FILTER_F0(fp);
        f1 = FILTER_F1(fp);
        if(f0 == target) r2_valid |= 1; // v<<1 survives
        if(f1 == target) r2_valid |= 2; // (v<<1)|1 survives
    }
    if(r1_valid & 2) { // v|1 survived R1
        fp = filter_pair_with_nib((v | 1) << 1, nib2);
        f0 = FILTER_F0(fp);
        f1 = FILTER_F1(fp);
        if(f0 == target) r2_valid |= 4; // (v|1)<<1 survives
        if(f1 == target) r2_valid |= 8; // ((v|1)<<1)|1 survives
    }

    if(!r2_valid) return 0;

    // Round 3: only check children of actual R2 survivors
    target = BIT(oks, 3);

    if(r2_valid & 1) { // v<<1 survived R2
        fp = filter_pair_with_nib(v << 2, nib3);
        f0 = FILTER_F0(fp);
        f1 = FILTER_F1(fp);
        if(f0 == target || f1 == target) return 1;
    }
    if(r2_valid & 2) { // (v<<1)|1 survived R2
        fp = filter_pair_with_nib((v << 2) | 2, nib3);
        f0 = FILTER_F0(fp);
        f1 = FILTER_F1(fp);
        if(f0 == target || f1 == target) return 1;
    }
    if(r2_valid & 4) { // (v|1)<<1 survived R2
        fp = filter_pair_with_nib((v | 1) << 2, nib3);
        f0 = FILTER_F0(fp);
        f1 = FILTER_F1(fp);
        if(f0 == target || f1 == target) return 1;
    }
    if(r2_valid & 8) { // ((v|1)<<1)|1 survived R2
        fp = filter_pair_with_nib(((v | 1) << 2) | 2, nib3);
        f0 = FILTER_F0(fp);
        f1 = FILTER_F1(fp);
        if(f0 == target || f1 == target) return 1;
    }

    return 0;
}

// 8x unrolled Duff's device for duplicate detection in packed 24-bit states
static inline __attribute__((always_inline)) bool scan_for_duplicate_8x(
    const uint8_t* states, // Packed 24-bit state array
    int count, // Number of entries to scan
    uint32_t state_masked) // Target state with MSB masked off (& 0x00FFFFFF)
{
    if(count <= 0) return false;

    const uint8_t* p = states;
    int n = (count + 7) / 8; // Number of 8-iteration chunks (rounded up)

    // Duff's Device: jump into unrolled loop based on remainder
    switch(count % 8) {
    case 0:
        do {
            if((*(uint32_t*)p & 0x00FFFFFF) == state_masked) return true;
            p += 3;
            __attribute__((fallthrough));
        case 7:
            if((*(uint32_t*)p & 0x00FFFFFF) == state_masked) return true;
            p += 3;
            __attribute__((fallthrough));
        case 6:
            if((*(uint32_t*)p & 0x00FFFFFF) == state_masked) return true;
            p += 3;
            __attribute__((fallthrough));
        case 5:
            if((*(uint32_t*)p & 0x00FFFFFF) == state_masked) return true;
            p += 3;
            __attribute__((fallthrough));
        case 4:
            if((*(uint32_t*)p & 0x00FFFFFF) == state_masked) return true;
            p += 3;
            __attribute__((fallthrough));
        case 3:
            if((*(uint32_t*)p & 0x00FFFFFF) == state_masked) return true;
            p += 3;
            __attribute__((fallthrough));
        case 2:
            if((*(uint32_t*)p & 0x00FFFFFF) == state_masked) return true;
            p += 3;
            __attribute__((fallthrough));
        case 1:
            if((*(uint32_t*)p & 0x00FFFFFF) == state_masked) return true;
            p += 3;
        } while(--n > 0);
    }
    return false;
}

#endif // MFKEY_DEDUP_H
