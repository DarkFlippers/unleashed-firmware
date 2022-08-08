#include "decoder_hid.h"
#include <furi_hal.h>

constexpr uint32_t clocks_in_us = 64;

constexpr uint32_t jitter_time_us = 20;
constexpr uint32_t min_time_us = 64;
constexpr uint32_t max_time_us = 80;

constexpr uint32_t min_time = (min_time_us - jitter_time_us) * clocks_in_us;
constexpr uint32_t mid_time = ((max_time_us - min_time_us) / 2 + min_time_us) * clocks_in_us;
constexpr uint32_t max_time = (max_time_us + jitter_time_us) * clocks_in_us;

bool DecoderHID::read(uint8_t* data, uint8_t data_size) {
    bool result = false;
    furi_assert(data_size >= 3);

    if(ready) {
        result = true;
        hid.decode(
            reinterpret_cast<const uint8_t*>(&stored_data), sizeof(uint32_t) * 3, data, data_size);
        ready = false;
    }

    return result;
}

void DecoderHID::process_front(bool polarity, uint32_t time) {
    if(ready) return;

    if(polarity == true) {
        last_pulse_time = time;
    } else {
        last_pulse_time += time;

        if(last_pulse_time > min_time && last_pulse_time < max_time) {
            bool pulse;

            if(last_pulse_time < mid_time) {
                // 6 pulses
                pulse = false;
            } else {
                // 5 pulses
                pulse = true;
            }

            if(last_pulse == pulse) {
                pulse_count++;

                if(pulse) {
                    if(pulse_count > 4) {
                        pulse_count = 0;
                        store_data(1);
                    }
                } else {
                    if(pulse_count > 5) {
                        pulse_count = 0;
                        store_data(0);
                    }
                }
            } else {
                if(last_pulse) {
                    if(pulse_count > 2) {
                        store_data(1);
                    }
                } else {
                    if(pulse_count > 3) {
                        store_data(0);
                    }
                }

                pulse_count = 0;
                last_pulse = pulse;
            }
        }
    }
}

DecoderHID::DecoderHID() {
    reset_state();
}

void DecoderHID::store_data(bool data) {
    stored_data[0] = (stored_data[0] << 1) | ((stored_data[1] >> 31) & 1);
    stored_data[1] = (stored_data[1] << 1) | ((stored_data[2] >> 31) & 1);
    stored_data[2] = (stored_data[2] << 1) | data;

    if(hid.can_be_decoded(reinterpret_cast<const uint8_t*>(&stored_data), sizeof(uint32_t) * 3)) {
        ready = true;
    }
}

void DecoderHID::reset_state() {
    last_pulse = false;
    pulse_count = 0;
    ready = false;
    last_pulse_time = 0;
}
