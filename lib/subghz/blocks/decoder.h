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

/**
 * Add data bit when decoding.
 * @param decoder Pointer to a SubGhzBlockDecoder instance
 * @param bit data, 1bit
 */
void subghz_protocol_blocks_add_bit(SubGhzBlockDecoder* decoder, uint8_t bit);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param decoder Pointer to a SubGhzBlockDecoder instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_blocks_get_hash_data(SubGhzBlockDecoder* decoder, size_t len);
