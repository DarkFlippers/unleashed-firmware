#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define bit_read(value, bit) (((value) >> (bit)) & 0x01)
#define bit_set(value, bit) ((value) |= (1UL << (bit)))
#define bit_clear(value, bit) ((value) &= ~(1UL << (bit)))
#define bit_write(value, bit, bitvalue) (bitvalue ? bit_set(value, bit) : bit_clear(value, bit))
#define DURATION_DIFF(x, y) ((x < y) ? (y - x) : (x - y))

/**
 * Flip the data bitwise.
 * @param key In data
 * @param count_bit number of data bits
 * @return Reverse data
 */
uint64_t subghz_protocol_blocks_reverse_key(uint64_t key, uint8_t count_bit);

/**
 * Get parity the data bitwise.
 * @param key In data
 * @param count_bit number of data bits
 * @return parity
 */
uint8_t subghz_protocol_blocks_get_parity(uint64_t key, uint8_t count_bit);
