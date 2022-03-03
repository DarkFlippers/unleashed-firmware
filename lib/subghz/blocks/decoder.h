#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct SubGhzBlockDecoder SubGhzBlockDecoder;

struct SubGhzBlockDecoder {
    uint32_t parser_step;
    uint32_t te_last;
    uint64_t decode_data;
    uint8_t decode_count_bit;
};

void subghz_protocol_blocks_add_bit(SubGhzBlockDecoder* decoder, uint8_t bit);
uint8_t subghz_protocol_blocks_get_hash_data(SubGhzBlockDecoder* decoder, size_t len);
