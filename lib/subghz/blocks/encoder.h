#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <lib/toolbox/level_duration.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool is_running;
    size_t repeat;
    size_t front;
    size_t size_upload;
    LevelDuration* upload;

} SubGhzProtocolBlockEncoder;

typedef enum {
    SubGhzProtocolBlockAlignBitLeft,
    SubGhzProtocolBlockAlignBitRight,
} SubGhzProtocolBlockAlignBit;

/**
 * Set data bit when encoding HEX array.
 * @param bit_value The value of the bit to be set
 * @param data_array Pointer to a HEX array
 * @param set_index_bit Number set a bit in the array starting from the left
 * @param max_size_array array size, check not to overflow
 */
void subghz_protocol_blocks_set_bit_array(
    bool bit_value,
    uint8_t data_array[],
    size_t set_index_bit,
    size_t max_size_array);

/**
 * Get data bit when encoding HEX array.
 * @param data_array Pointer to a HEX array
 * @param read_index_bit Number get a bit in the array starting from the left
 * @return bool value bit
 */
bool subghz_protocol_blocks_get_bit_array(uint8_t data_array[], size_t read_index_bit);

/**
 * Generating an upload from data.
 * @param data_array Pointer to a HEX array
 * @param count_bit_data_array How many bits in the array are processed
 * @param upload Pointer to a LevelDuration
 * @param max_size_upload upload size, check not to overflow
 * @param duration_bit duration 1 bit
 * @param align_bit alignment of useful bits in an array
 */
size_t subghz_protocol_blocks_get_upload_from_bit_array(
    uint8_t data_array[],
    size_t count_bit_data_array,
    LevelDuration* upload,
    size_t max_size_upload,
    uint32_t duration_bit,
    SubGhzProtocolBlockAlignBit align_bit);

#ifdef __cplusplus
}
#endif