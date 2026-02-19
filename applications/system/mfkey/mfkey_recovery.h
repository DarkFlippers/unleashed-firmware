#ifndef MFKEY_RECOVERY_H
#define MFKEY_RECOVERY_H

#include "mfkey.h"
#include "crypto1.h"

// Scratch buffer for radix sort. Also reused as ping-pong buffer by
// state_loop_r4 during expansion (phases don't overlap).
extern unsigned int g_radix_temp[1280];

// Radix sort for 24-bit packed states (O(n) non-recursive sorting)
void radix_sort_32(unsigned int* arr, int low, int high, unsigned int* temp);

// Bitsliced recovery: 32-way parallel verification
// Returns: >0 = states checked, -1 = key found, -2 = aborted
int old_recover_bs(
    unsigned int odd[],
    int o_head,
    int o_tail,
    int oks,
    unsigned int even[],
    int e_head,
    int e_tail,
    int eks,
    int rem,
    int s,
    MfClassicNonce* n,
    unsigned int in,
    int first_run);

#endif // MFKEY_RECOVERY_H
