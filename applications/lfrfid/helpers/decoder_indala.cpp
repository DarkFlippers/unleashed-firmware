#include "decoder_indala.h"
#include <furi_hal.h>

constexpr uint32_t clocks_in_us = 64;
constexpr uint32_t us_per_bit = 255;

bool DecoderIndala::read(uint8_t* data, uint8_t data_size) {
    bool result = false;

    if(ready) {
        result = true;
        if(cursed_data_valid) {
            indala.decode(
                reinterpret_cast<const uint8_t*>(&cursed_raw_data),
                sizeof(uint64_t),
                data,
                data_size);
        } else {
            indala.decode(
                reinterpret_cast<const uint8_t*>(&raw_data), sizeof(uint64_t), data, data_size);
        }
        reset_state();
    }

    return result;
}

void DecoderIndala::process_front(bool polarity, uint32_t time) {
    if(ready) return;

    process_internal(polarity, time, &raw_data);
    if(ready) return;

    if(polarity) {
        time = time + 110;
    } else {
        time = time - 110;
    }

    process_internal(!polarity, time, &cursed_raw_data);
    if(ready) {
        cursed_data_valid = true;
    }
}

void DecoderIndala::process_internal(bool polarity, uint32_t time, uint64_t* data) {
    time /= clocks_in_us;
    time += (us_per_bit / 2);

    uint32_t bit_count = (time / us_per_bit);

    if(bit_count < 64) {
        for(uint32_t i = 0; i < bit_count; i++) {
            *data = (*data << 1) | polarity;

            if((*data >> 32) == 0xa0000000ULL) {
                if(indala.can_be_decoded(
                       reinterpret_cast<const uint8_t*>(data), sizeof(uint64_t))) {
                    ready = true;
                    break;
                }
            }
        }
    }
}

DecoderIndala::DecoderIndala() {
    reset_state();
}

void DecoderIndala::reset_state() {
    raw_data = 0;
    cursed_raw_data = 0;
    ready = false;
    cursed_data_valid = false;
}
