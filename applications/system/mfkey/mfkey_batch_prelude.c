// MFKey Batch Prelude - 32-lane parallel tree expansion through rounds 0-3
// Extracted from mfkey_attack.c for better code organization

#pragma GCC optimize("O3")

#include "mfkey_batch_prelude.h"
#include "crypto1.h"
#include "mfkey_dedup.h"
#include <string.h>

// Precomputed filter LUTs for shared input values 0..7
// Derived from 0xEC57E80A stride 8 (lookup2 outputs even values, nibble adds 0 or 1)
static const uint8_t FILTER_LUT_TABLE[8] = {0x4, 0x5, 0xC, 0xB, 0x4, 0xA, 0xE, 0xA};

// Super-LUT: Combined bitmasks indexed by full LUT value (0-15)
// Eliminates conditional branches - single indexed load per round
// Total: 2KB (8×16 + 4×2×16 + 2×4×16 + 8×16 = 512 uint32_t)

// Round 0: 8 offsets × 16 LUT values (regenerated from filter() brute-force)
static const uint32_t R0_COMBINED[8][16] = {
    {
        0x00000000,
        0x0DD30DD3,
        0x00000000,
        0x0DD30DD3,
        0xF22CF22C,
        0xFFFFFFFF,
        0xF22CF22C,
        0xFFFFFFFF,
        0x00000000,
        0x0DD30DD3,
        0x00000000,
        0x0DD30DD3,
        0xF22CF22C,
        0xFFFFFFFF,
        0xF22CF22C,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x00000DD3,
        0x0DD30000,
        0x0DD30DD3,
        0x0000F22C,
        0x0000FFFF,
        0x0DD3F22C,
        0x0DD3FFFF,
        0xF22C0000,
        0xF22C0DD3,
        0xFFFF0000,
        0xFFFF0DD3,
        0xF22CF22C,
        0xF22CFFFF,
        0xFFFFF22C,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x00000000,
        0x0DD30DD3,
        0x0DD30DD3,
        0x00000000,
        0x00000000,
        0x0DD30DD3,
        0x0DD30DD3,
        0xF22CF22C,
        0xF22CF22C,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xF22CF22C,
        0xF22CF22C,
        0xFFFFFFFF,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x0DD30DD3,
        0x00000000,
        0x0DD30DD3,
        0xF22CF22C,
        0xFFFFFFFF,
        0xF22CF22C,
        0xFFFFFFFF,
        0x00000000,
        0x0DD30DD3,
        0x00000000,
        0x0DD30DD3,
        0xF22CF22C,
        0xFFFFFFFF,
        0xF22CF22C,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x0DD30000,
        0x00000DD3,
        0x0DD30DD3,
        0xF22C0000,
        0xFFFF0000,
        0xF22C0DD3,
        0xFFFF0DD3,
        0x0000F22C,
        0x0DD3F22C,
        0x0000FFFF,
        0x0DD3FFFF,
        0xF22CF22C,
        0xFFFFF22C,
        0xF22CFFFF,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x00000DD3,
        0x0DD30000,
        0x0DD30DD3,
        0x0000F22C,
        0x0000FFFF,
        0x0DD3F22C,
        0x0DD3FFFF,
        0xF22C0000,
        0xF22C0DD3,
        0xFFFF0000,
        0xFFFF0DD3,
        0xF22CF22C,
        0xF22CFFFF,
        0xFFFFF22C,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x0DD30000,
        0x00000DD3,
        0x0DD30DD3,
        0xF22C0000,
        0xFFFF0000,
        0xF22C0DD3,
        0xFFFF0DD3,
        0x0000F22C,
        0x0DD3F22C,
        0x0000FFFF,
        0x0DD3FFFF,
        0xF22CF22C,
        0xFFFFF22C,
        0xF22CFFFF,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x00000000,
        0x0DD30DD3,
        0x0DD30DD3,
        0x00000000,
        0x00000000,
        0x0DD30DD3,
        0x0DD30DD3,
        0xF22CF22C,
        0xF22CF22C,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xF22CF22C,
        0xF22CF22C,
        0xFFFFFFFF,
        0xFFFFFFFF,
    },
};

// Round 1: 4 offsets × 2 children × 16 LUT values (regenerated from filter() brute-force)
static const uint32_t R1_COMBINED[4][2][16] = {
    {
        {
            0x00000000,
            0x003D3D3D,
            0x3D000000,
            0x3D3D3D3D,
            0x00C2C2C2,
            0x00FFFFFF,
            0x3DC2C2C2,
            0x3DFFFFFF,
            0xC2000000,
            0xC23D3D3D,
            0xFF000000,
            0xFF3D3D3D,
            0xC2C2C2C2,
            0xC2FFFFFF,
            0xFFC2C2C2,
            0xFFFFFFFF,
        },
        {
            0x00000000,
            0x00292929,
            0x29000000,
            0x29292929,
            0x00D6D6D6,
            0x00FFFFFF,
            0x29D6D6D6,
            0x29FFFFFF,
            0xD6000000,
            0xD6292929,
            0xFF000000,
            0xFF292929,
            0xD6D6D6D6,
            0xD6FFFFFF,
            0xFFD6D6D6,
            0xFFFFFFFF,
        },
    },
    {
        {
            0x00000000,
            0x3D3D0000,
            0x00003D3D,
            0x3D3D3D3D,
            0xC2C20000,
            0xFFFF0000,
            0xC2C23D3D,
            0xFFFF3D3D,
            0x0000C2C2,
            0x3D3DC2C2,
            0x0000FFFF,
            0x3D3DFFFF,
            0xC2C2C2C2,
            0xFFFFC2C2,
            0xC2C2FFFF,
            0xFFFFFFFF,
        },
        {
            0x00000000,
            0x29290000,
            0x00002929,
            0x29292929,
            0xD6D60000,
            0xFFFF0000,
            0xD6D62929,
            0xFFFF2929,
            0x0000D6D6,
            0x2929D6D6,
            0x0000FFFF,
            0x2929FFFF,
            0xD6D6D6D6,
            0xFFFFD6D6,
            0xD6D6FFFF,
            0xFFFFFFFF,
        },
    },
    {
        {
            0x00000000,
            0x003D3D00,
            0x3D00003D,
            0x3D3D3D3D,
            0x00C2C200,
            0x00FFFF00,
            0x3DC2C23D,
            0x3DFFFF3D,
            0xC20000C2,
            0xC23D3DC2,
            0xFF0000FF,
            0xFF3D3DFF,
            0xC2C2C2C2,
            0xC2FFFFC2,
            0xFFC2C2FF,
            0xFFFFFFFF,
        },
        {
            0x00000000,
            0x00292900,
            0x29000029,
            0x29292929,
            0x00D6D600,
            0x00FFFF00,
            0x29D6D629,
            0x29FFFF29,
            0xD60000D6,
            0xD62929D6,
            0xFF0000FF,
            0xFF2929FF,
            0xD6D6D6D6,
            0xD6FFFFD6,
            0xFFD6D6FF,
            0xFFFFFFFF,
        },
    },
    {
        {
            0x00000000,
            0x00003D00,
            0x3D3D003D,
            0x3D3D3D3D,
            0x0000C200,
            0x0000FF00,
            0x3D3DC23D,
            0x3D3DFF3D,
            0xC2C200C2,
            0xC2C23DC2,
            0xFFFF00FF,
            0xFFFF3DFF,
            0xC2C2C2C2,
            0xC2C2FFC2,
            0xFFFFC2FF,
            0xFFFFFFFF,
        },
        {
            0x00000000,
            0x00002900,
            0x29290029,
            0x29292929,
            0x0000D600,
            0x0000FF00,
            0x2929D629,
            0x2929FF29,
            0xD6D600D6,
            0xD6D629D6,
            0xFFFF00FF,
            0xFFFF29FF,
            0xD6D6D6D6,
            0xD6D6FFD6,
            0xFFFFD6FF,
            0xFFFFFFFF,
        },
    },
};

// Round 2: 2 offsets × 4 children × 16 LUT values (regenerated from filter() brute-force)
static const uint32_t R2_COMBINED[2][4][16] = {
    {
        {
            0x00000000,
            0x77000777,
            0x00777000,
            0x77777777,
            0x88000888,
            0xFF000FFF,
            0x88777888,
            0xFF777FFF,
            0x00888000,
            0x77888777,
            0x00FFF000,
            0x77FFF777,
            0x88888888,
            0xFF888FFF,
            0x88FFF888,
            0xFFFFFFFF,
        },
        {
            0x00000000,
            0x11000111,
            0x00111000,
            0x11111111,
            0xEE000EEE,
            0xFF000FFF,
            0xEE111EEE,
            0xFF111FFF,
            0x00EEE000,
            0x11EEE111,
            0x00FFF000,
            0x11FFF111,
            0xEEEEEEEE,
            0xFFEEEFFF,
            0xEEFFFEEE,
            0xFFFFFFFF,
        },
        {
            0x00000000,
            0x66000666,
            0x00666000,
            0x66666666,
            0x99000999,
            0xFF000FFF,
            0x99666999,
            0xFF666FFF,
            0x00999000,
            0x66999666,
            0x00FFF000,
            0x66FFF666,
            0x99999999,
            0xFF999FFF,
            0x99FFF999,
            0xFFFFFFFF,
        },
        {
            0x00000000,
            0x66000666,
            0x00666000,
            0x66666666,
            0x99000999,
            0xFF000FFF,
            0x99666999,
            0xFF666FFF,
            0x00999000,
            0x66999666,
            0x00FFF000,
            0x66FFF666,
            0x99999999,
            0xFF999FFF,
            0x99FFF999,
            0xFFFFFFFF,
        },
    },
    {
        {
            0x00000000,
            0x00700770,
            0x77077007,
            0x77777777,
            0x00800880,
            0x00F00FF0,
            0x77877887,
            0x77F77FF7,
            0x88088008,
            0x88788778,
            0xFF0FF00F,
            0xFF7FF77F,
            0x88888888,
            0x88F88FF8,
            0xFF8FF88F,
            0xFFFFFFFF,
        },
        {
            0x00000000,
            0x00100110,
            0x11011001,
            0x11111111,
            0x00E00EE0,
            0x00F00FF0,
            0x11E11EE1,
            0x11F11FF1,
            0xEE0EE00E,
            0xEE1EE11E,
            0xFF0FF00F,
            0xFF1FF11F,
            0xEEEEEEEE,
            0xEEFEEFFE,
            0xFFEFFEEF,
            0xFFFFFFFF,
        },
        {
            0x00000000,
            0x00600660,
            0x66066006,
            0x66666666,
            0x00900990,
            0x00F00FF0,
            0x66966996,
            0x66F66FF6,
            0x99099009,
            0x99699669,
            0xFF0FF00F,
            0xFF6FF66F,
            0x99999999,
            0x99F99FF9,
            0xFF9FF99F,
            0xFFFFFFFF,
        },
        {
            0x00000000,
            0x00600660,
            0x66066006,
            0x66666666,
            0x00900990,
            0x00F00FF0,
            0x66966996,
            0x66F66FF6,
            0x99099009,
            0x99699669,
            0xFF0FF00F,
            0xFF6FF66F,
            0x99999999,
            0x99F99FF9,
            0xFF9FF99F,
            0xFFFFFFFF,
        },
    },
};

// Round 3: Individual child masks for target=1 — [child_idx][lut]
// child_idx c encodes R1-R2-R3 binary choices (b0*4 + b1*2 + b2)
// For target=0: use ~mask at runtime (complement)
// Replaces R3_PAIRED: same total size (512 bytes)
// Verification: R3_PAIRED[1][i][lut] == R3_INDIVIDUAL[2*i][lut] | R3_INDIVIDUAL[2*i+1][lut]
static const uint32_t R3_INDIVIDUAL[8][16] = {
    {
        0x00000000,
        0x0C3CF03F,
        0xF3C30FC0,
        0xFFFFFFFF,
        0x00000000,
        0x0C3CF03F,
        0xF3C30FC0,
        0xFFFFFFFF,
        0x00000000,
        0x0C3CF03F,
        0xF3C30FC0,
        0xFFFFFFFF,
        0x00000000,
        0x0C3CF03F,
        0xF3C30FC0,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x04145015,
        0x51410540,
        0x55555555,
        0x0828A02A,
        0x0C3CF03F,
        0x5969A56A,
        0x5D7DF57F,
        0xA2820A80,
        0xA6965A95,
        0xF3C30FC0,
        0xF7D75FD5,
        0xAAAAAAAA,
        0xAEBEFABF,
        0xFBEBAFEA,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x0828A02A,
        0xA2820A80,
        0xAAAAAAAA,
        0x04145015,
        0x0C3CF03F,
        0xA6965A95,
        0xAEBEFABF,
        0x51410540,
        0x5969A56A,
        0xF3C30FC0,
        0xFBEBAFEA,
        0x55555555,
        0x5D7DF57F,
        0xF7D75FD5,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x0828A02A,
        0xA2820A80,
        0xAAAAAAAA,
        0x04145015,
        0x0C3CF03F,
        0xA6965A95,
        0xAEBEFABF,
        0x51410540,
        0x5969A56A,
        0xF3C30FC0,
        0xFBEBAFEA,
        0x55555555,
        0x5D7DF57F,
        0xF7D75FD5,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x04145015,
        0x51410540,
        0x55555555,
        0x0828A02A,
        0x0C3CF03F,
        0x5969A56A,
        0x5D7DF57F,
        0xA2820A80,
        0xA6965A95,
        0xF3C30FC0,
        0xF7D75FD5,
        0xAAAAAAAA,
        0xAEBEFABF,
        0xFBEBAFEA,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x0C3CF03F,
        0x0C3CF03F,
        0x0C3CF03F,
        0x0C3CF03F,
        0xF3C30FC0,
        0xF3C30FC0,
        0xF3C30FC0,
        0xF3C30FC0,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x04145015,
        0x51410540,
        0x55555555,
        0x0828A02A,
        0x0C3CF03F,
        0x5969A56A,
        0x5D7DF57F,
        0xA2820A80,
        0xA6965A95,
        0xF3C30FC0,
        0xF7D75FD5,
        0xAAAAAAAA,
        0xAEBEFABF,
        0xFBEBAFEA,
        0xFFFFFFFF,
    },
    {
        0x00000000,
        0x04145015,
        0x51410540,
        0x55555555,
        0x0828A02A,
        0x0C3CF03F,
        0x5969A56A,
        0x5D7DF57F,
        0xA2820A80,
        0xA6965A95,
        0xF3C30FC0,
        0xF7D75FD5,
        0xAAAAAAAA,
        0xAEBEFABF,
        0xFBEBAFEA,
        0xFFFFFFFF,
    },
};

// Combined prefilter + reconstruction in one pass.
// leaf_masks[c] (c=0..7) gives lanes where child c survives R0-R3+R4.
// Return value is OR of all 8 leaf_masks (= lane survival mask).
uint32_t
    batch_prelude_unified(uint32_t batch_base, int oks, uint32_t r4_mask, uint32_t leaf_masks[8]) {
    OPT_BARRIER(oks);

    // --- ROUND 0 ---
    uint32_t idx_hi = (batch_base >> 8) & 0xFF;
    uint32_t idx_nib = (batch_base >> 16) & 0xF;
    uint32_t shared = lookup2[idx_hi] | ((0x0d938 >> idx_nib) & 1);
    int lut = FILTER_LUT_TABLE[shared];
    uint32_t off0 = (batch_base >> 5) & 7;

    uint32_t valid = R0_COMBINED[off0][lut];
    if((oks & 1) == 0) valid = ~valid;
    if(!valid) return 0;

    // --- ROUND 1 ---
    int target = BIT(oks, 1);
    uint32_t node_base = (batch_base << 1);
    shared = lookup2[(node_base >> 8) & 0xFF] | ((0x0d938 >> ((node_base >> 16) & 0xF)) & 1);
    lut = FILTER_LUT_TABLE[shared];
    uint32_t off1 = (batch_base >> 5) & 3;

    uint32_t p0 = R1_COMBINED[off1][0][lut];
    uint32_t p1 = R1_COMBINED[off1][1][lut];
    if(target == 0) {
        p0 = ~p0;
        p1 = ~p1;
    }
    p0 &= valid;
    p1 &= valid;
    if(!(p0 | p1)) return 0;

    // --- ROUND 2 ---
    target = BIT(oks, 2);
    node_base = (batch_base << 2);
    shared = lookup2[(node_base >> 8) & 0xFF] | ((0x0d938 >> ((node_base >> 16) & 0xF)) & 1);
    lut = FILTER_LUT_TABLE[shared];
    uint32_t off2 = (batch_base >> 5) & 1;

    uint32_t p00 = R2_COMBINED[off2][0][lut];
    uint32_t p01 = R2_COMBINED[off2][1][lut];
    uint32_t p10 = R2_COMBINED[off2][2][lut];
    uint32_t p11 = R2_COMBINED[off2][3][lut];
    if(target == 0) {
        p00 = ~p00;
        p01 = ~p01;
        p10 = ~p10;
        p11 = ~p11;
    }
    p00 &= p0;
    p01 &= p0;
    p10 &= p1;
    p11 &= p1;
    if(!(p00 | p01 | p10 | p11)) return 0;

    // --- ROUND 3 (individual child masks) ---
    node_base = (batch_base << 3);
    shared = lookup2[(node_base >> 8) & 0xFF] | ((0x0d938 >> ((node_base >> 16) & 0xF)) & 1);
    lut = FILTER_LUT_TABLE[shared];

    int target3 = BIT(oks, 3);

    // Load 8 individual child masks, apply target complement, AND with parent + r4_mask
    // Child c: parent is p[c>>2][c&2 ? 1 : 0] → parent index from R1 bit (c>>2) and R2 bit ((c>>1)&1)
    // Parent mapping: c=0,1→p00  c=2,3→p01  c=4,5→p10  c=6,7→p11
    const uint32_t parents[4] = {p00, p01, p10, p11};
    uint32_t all = 0;
    for(int c = 0; c < 8; c++) {
        uint32_t m = R3_INDIVIDUAL[c][lut];
        if(target3 == 0) m = ~m;
        m &= parents[c >> 1];
        m &= r4_mask;
        leaf_masks[c] = m;
        all |= m;
    }

    return all;
}
