#include "encoder.h"
#include "math.h"
#include <core/check.h>

#define TAG "SubGhzBlockEncoder"

void subghz_protocol_blocks_set_bit_array(
    bool bit_value,
    uint8_t data_array[],
    size_t set_index_bit,
    size_t max_size_array) {
    furi_assert(set_index_bit < max_size_array * 8);
    bit_write(data_array[set_index_bit >> 3], 7 - (set_index_bit & 0x7), bit_value);
}

bool subghz_protocol_blocks_get_bit_array(uint8_t data_array[], size_t read_index_bit) {
    return bit_read(data_array[read_index_bit >> 3], 7 - (read_index_bit & 0x7));
}

size_t subghz_protocol_blocks_get_upload(
    uint8_t data_array[],
    size_t count_bit_data_array,
    LevelDuration* upload,
    size_t max_size_upload,
    uint32_t duration_bit) {
    size_t index_bit = 0;
    size_t size_upload = 0;
    uint32_t duration = duration_bit;
    bool last_bit = subghz_protocol_blocks_get_bit_array(data_array, index_bit++);
    for(size_t i = 1; i < count_bit_data_array; i++) {
        if(last_bit == subghz_protocol_blocks_get_bit_array(data_array, index_bit)) {
            duration += duration_bit;
        } else {
            furi_assert(max_size_upload > size_upload);
            upload[size_upload++] = level_duration_make(
                subghz_protocol_blocks_get_bit_array(data_array, index_bit - 1), duration);
            last_bit = !last_bit;
            duration = duration_bit;
        }
        index_bit++;
    }
    upload[size_upload++] = level_duration_make(
        subghz_protocol_blocks_get_bit_array(data_array, index_bit - 1), duration);
    return size_upload;
}
