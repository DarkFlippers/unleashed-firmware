// MFKey State Expansion - LFSR tree search through rounds 1-12
// Extracted from mfkey_attack.c for better code organization

#pragma GCC optimize("O3")

#include "mfkey_state_expansion.h"
#include "mfkey_recovery.h"
#include "crypto1.h"
#include <stdint.h>

#define OPT_BARRIER(x) __asm__ volatile("" : "+r"(x))

// Pre-fold xks_bit into the filter constant so BIT(adj, idx) == BIT(0xEC57E80A, idx) ^ xks_bit
#define ADJ_FILTER(xks, round) (0xEC57E80Au ^ (-(uint32_t)BIT(xks, round)))

// In-place LFSR expansion for rounds 4-12, called after batch prelude completes rounds 0-3.
// fork_delta deferred past round 4 early-exit (~95% of calls exit at round 4).
__attribute__((hot)) int state_loop_r4(
    unsigned int* states_buffer,
    int count, // Number of active states in buffer
    int xks,
    int m1,
    int m2,
    unsigned int in,
    int and_val) {
    OPT_BARRIER(xks); // Prevent hoisting of BIT(xks, round) extractions
    if(count == 0) return -1;

    // Round 4 (in-place, no parity update)
    int states_tail = count - 1;
    {
        uint32_t adj = ADJ_FILTER(xks, 4);
        for(int s = 0; s <= states_tail; ++s) {
            unsigned int raw = states_buffer[s];
            OPT_BARRIER(raw);
            unsigned int v = raw << 1;
            uint32_t fp = filter_pair_xor(v, adj);
            int f0_x = FILTER_F0(fp);
            int f1_x = FILTER_F1(fp);

            if(__builtin_expect((f0_x ^ f1_x) != 0, 0)) {
                // Single child survives
                states_buffer[s] = v | f0_x;
            } else if(__builtin_expect(f0_x == 0, 1)) {
                // Both children survive — fork
                states_buffer[++states_tail] = states_buffer[s + 1];
                states_buffer[s] = v;
                s++;
                states_buffer[s] = v | 1;
            } else {
                // Neither survives — eliminate
                states_buffer[s--] = states_buffer[states_tail--];
            }
        }
        if(__builtin_expect(states_tail < 0, 1)) return -1;
    }

    // Fork delta deferred past round 4 early-exit (~95% of calls don't reach here)
    uint32_t fork_delta = ((m1 & 1) << 25) | ((m2 & 1) << 24) | 1;

    // Round 5 (unrolled, in-place)
    {
        uint32_t adj = ADJ_FILTER(xks, 5);
        unsigned int r5_in = ((in >> 2) & and_val) << 24;
        for(int s = 0; s <= states_tail; ++s) {
            unsigned int raw = states_buffer[s];
            OPT_BARRIER(raw);
            unsigned int v = raw << 1;
            uint32_t fp = filter_pair_xor(v, adj);
            int f0_x = FILTER_F0(fp);
            int f1_x = FILTER_F1(fp);

            if(__builtin_expect((f0_x ^ f1_x) != 0, 0)) {
                v |= f0_x;
                OPT_BARRIER(v);
                states_buffer[s] = update_contribution_reg(v, m1, m2) ^ r5_in;
            } else if(__builtin_expect(f0_x == 0, 1)) {
                states_buffer[++states_tail] = states_buffer[s + 1];
                OPT_BARRIER(v);
                uint32_t p0 = update_contribution_reg(v, m1, m2) ^ r5_in;
                states_buffer[s] = p0;
                s++;
                states_buffer[s] = p0 ^ fork_delta;
            } else {
                states_buffer[s--] = states_buffer[states_tail--];
            }
        }
        if(__builtin_expect(states_tail < 0, 0)) return -1;
    }

    // Round 6 (unrolled, in-place)
    {
        uint32_t adj = ADJ_FILTER(xks, 6);
        unsigned int r6_in = ((in >> 4) & and_val) << 24;
        for(int s = 0; s <= states_tail; ++s) {
            unsigned int raw = states_buffer[s];
            OPT_BARRIER(raw);
            unsigned int v = raw << 1;
            uint32_t fp = filter_pair_xor(v, adj);
            int f0_x = FILTER_F0(fp);
            int f1_x = FILTER_F1(fp);

            if(__builtin_expect((f0_x ^ f1_x) != 0, 0)) {
                v |= f0_x;
                OPT_BARRIER(v);
                states_buffer[s] = update_contribution_reg(v, m1, m2) ^ r6_in;
            } else if(__builtin_expect(f0_x == 0, 1)) {
                states_buffer[++states_tail] = states_buffer[s + 1];
                OPT_BARRIER(v);
                uint32_t p0 = update_contribution_reg(v, m1, m2) ^ r6_in;
                states_buffer[s] = p0;
                s++;
                states_buffer[s] = p0 ^ fork_delta;
            } else {
                states_buffer[s--] = states_buffer[states_tail--];
            }
        }
        if(__builtin_expect(states_tail < 0, 0)) return -1;
    }

    // Rounds 7-12 (loop, in-place)
    for(int round = 7; round <= 12; ++round) {
        uint32_t adj = ADJ_FILTER(xks, round);
        unsigned int round_in = ((in >> (2 * (round - 4))) & and_val) << 24;
        for(int s = 0; s <= states_tail; ++s) {
            unsigned int raw = states_buffer[s];
            OPT_BARRIER(raw);
            unsigned int v = raw << 1;
            uint32_t fp = filter_pair_xor(v, adj);
            int f0_x = FILTER_F0(fp);
            int f1_x = FILTER_F1(fp);

            if(__builtin_expect((f0_x ^ f1_x) != 0, 0)) {
                v |= f0_x;
                OPT_BARRIER(v);
                states_buffer[s] = update_contribution_reg(v, m1, m2) ^ round_in;
            } else if(__builtin_expect(f0_x == 0, 1)) {
                states_buffer[++states_tail] = states_buffer[s + 1];
                OPT_BARRIER(v);
                uint32_t p0 = update_contribution_reg(v, m1, m2) ^ round_in;
                states_buffer[s] = p0;
                s++;
                states_buffer[s] = p0 ^ fork_delta;
            } else {
                states_buffer[s--] = states_buffer[states_tail--];
            }
        }
        if(__builtin_expect(states_tail < 0, 0)) return -1;
    }

    return states_tail;
}
