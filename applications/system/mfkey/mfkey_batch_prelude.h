#ifndef MFKEY_BATCH_PRELUDE_H
#define MFKEY_BATCH_PRELUDE_H

#include <stdint.h>
#include "mfkey.h"

// Returns lane survival mask AND per-child leaf masks.
// leaf_masks[c] (c=0..7) gives lanes where child c survives R0-R3+R4.
uint32_t
    batch_prelude_unified(uint32_t batch_base, int oks, uint32_t r4_mask, uint32_t leaf_masks[8]);

#endif // MFKEY_BATCH_PRELUDE_H
