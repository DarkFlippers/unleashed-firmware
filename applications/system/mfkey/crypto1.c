#pragma GCC optimize("O3")
#pragma GCC optimize("-funroll-all-loops")

#include <inttypes.h>
#include "crypto1.h"
#include "mfkey.h"

#define BIT(x, n) ((x) >> (n) & 1)

void crypto1_get_lfsr(struct Crypto1State* state, MfClassicKey* lfsr) {
    int i;
    uint64_t lfsr_value = 0;
    for(i = 23; i >= 0; --i) {
        lfsr_value = lfsr_value << 1 | BIT(state->odd, i ^ 3);
        lfsr_value = lfsr_value << 1 | BIT(state->even, i ^ 3);
    }

    // Assign the key value to the MfClassicKey struct
    for(i = 0; i < 6; ++i) {
        lfsr->data[i] = (lfsr_value >> ((5 - i) * 8)) & 0xFF;
    }
}
