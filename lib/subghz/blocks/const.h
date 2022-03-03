#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    const uint16_t te_long;
    const uint16_t te_short;
    const uint16_t te_delta;
    const uint8_t min_count_bit_for_found;
} SubGhzBlockConst;
