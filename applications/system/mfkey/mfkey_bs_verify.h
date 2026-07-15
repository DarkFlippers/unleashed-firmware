#ifndef MFKEY_BS_VERIFY_H
#define MFKEY_BS_VERIFY_H

// Bit-sliced verification for all attack types (mfkey32, static_nested,
// static_encrypted). Processes 32 candidate LFSR states in parallel using
// SWAR (SIMD Within A Register) techniques.

#include "mfkey.h"
#include <stdint.h>
#include <stdbool.h>

#define BS_BATCH_SIZE 32

typedef struct {
    uint32_t odd[BS_BATCH_SIZE]; // Odd half-states (24-bit, lower bits used)
    uint32_t even[BS_BATCH_SIZE]; // Even half-states (24-bit)
    int count; // Current fill level (0-32)
} BsCandidateBatch;

// Mirrored runway buffer: bits 0-23 mirrored to 24-47.
// Max read index: head (0-23) + max tap offset (23) = 46 < 48.
typedef struct {
    uint32_t odd[48]; // 24 LFSR bits mirrored 2 times to avoid bounds checks
    uint32_t even[48]; // Each element holds 32 bits (one per lane)
    uint32_t odd_head; // Current head position (0-23 normalized)
    uint32_t even_head;
} Crypto1BitSlice;

// Initialize empty batch
static inline void bs_batch_init(BsCandidateBatch* batch) {
    batch->count = 0;
}

// Add candidate to batch
// Returns true if batch is now full (caller should verify and reset)
static inline bool bs_batch_add(BsCandidateBatch* batch, uint32_t odd, uint32_t even) {
    if(batch->count < BS_BATCH_SIZE) {
        batch->odd[batch->count] = odd;
        batch->even[batch->count] = even;
        batch->count++;
    }
    return batch->count >= BS_BATCH_SIZE;
}

// Initialize bitsliced state from candidate batch (transpose scalar → SWAR)
// Call once before dispatching to any verification kernel
void bs_init_from_candidates(Crypto1BitSlice* bs, const BsCandidateBatch* batch);

// Verify batch of up to 32 candidates against nonce data
// bs: pre-initialized bitsliced state (from bs_init_from_candidates)
// alive: lane mask with zero-states already rejected
// Returns bitmask where bit i is set if candidate i is valid
uint32_t bs_verify_batch_32(Crypto1BitSlice* bs, MfClassicNonce* nonce, uint32_t alive);

// Extract key from valid lane (unified for all attack types)
// lane = __builtin_ctz(valid_mask) to get first valid lane
void bs_extract_key(const BsCandidateBatch* batch, int lane, MfClassicNonce* nonce);

// Static nested verification: rollback uid_xor_nt1 → forward crypt uid_xor_nt0
// Returns bitmask of valid lanes (first-match)
uint32_t bs_verify_batch_32_nested(Crypto1BitSlice* bs, MfClassicNonce* nonce, uint32_t alive);

// Static encrypted verification: rollback uid_xor_nt0 + parity check
// Returns bitmask of ALL valid lanes (multi-result)
uint32_t bs_verify_batch_32_encrypted(
    Crypto1BitSlice* bs,
    const BsCandidateBatch* batch,
    MfClassicNonce* nonce,
    uint32_t alive);

#endif // MFKEY_BS_VERIFY_H
