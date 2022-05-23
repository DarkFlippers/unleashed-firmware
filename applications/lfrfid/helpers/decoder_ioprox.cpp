#include "decoder_ioprox.h"
#include <furi_hal.h>
#include <cli/cli.h>
#include <utility>

constexpr uint32_t clocks_in_us = 64;

constexpr uint32_t jitter_time_us = 20;
constexpr uint32_t min_time_us = 64;
constexpr uint32_t max_time_us = 80;
constexpr uint32_t baud_time_us = 500;

constexpr uint32_t min_time = (min_time_us - jitter_time_us) * clocks_in_us;
constexpr uint32_t mid_time = ((max_time_us - min_time_us) / 2 + min_time_us) * clocks_in_us;
constexpr uint32_t max_time = (max_time_us + jitter_time_us) * clocks_in_us;
constexpr uint32_t baud_time = baud_time_us * clocks_in_us;

bool DecoderIoProx::read(uint8_t* data, uint8_t data_size) {
    bool result = false;
    furi_assert(data_size >= 4);

    if(ready) {
        result = true;
        ioprox.decode(raw_data, sizeof(raw_data), data, data_size);
        ready = false;
    }

    return result;
}

void DecoderIoProx::process_front(bool is_rising_edge, uint32_t time) {
    if(ready) {
        return;
    }

    // Always track the time that's gone by.
    current_period_duration += time;
    demodulation_sample_duration += time;

    // If a baud time has elapsed, we're at a sample point.
    if(demodulation_sample_duration >= baud_time) {
        // Start a new baud period...
        demodulation_sample_duration = 0;
        demodulated_value_invalid = false;

        // ... and if we didn't have any baud errors, capture a sample.
        if(!demodulated_value_invalid) {
            store_data(current_demodulated_value);
        }
    }

    //
    // FSK demodulator.
    //

    // If this isn't a rising edge, this isn't a pulse of interest.
    // We're done.
    if(!is_rising_edge) {
        return;
    }

    bool is_valid_low = (current_period_duration > min_time) &&
                        (current_period_duration <= mid_time);
    bool is_valid_high = (current_period_duration > mid_time) &&
                         (current_period_duration < max_time);

    // If this is between the minimum and our threshold, this is a logical 0.
    if(is_valid_low) {
        current_demodulated_value = false;
    }
    // Otherwise, if between our threshold and the max time, it's a logical 1.
    else if(is_valid_high) {
        current_demodulated_value = true;
    }
    // Otherwise, invalidate this sample.
    else {
        demodulated_value_invalid = true;
    }

    // We're starting a new period; track that.
    current_period_duration = 0;
}

DecoderIoProx::DecoderIoProx() {
    reset_state();
}

void DecoderIoProx::store_data(bool data) {
    for(int i = 0; i < 7; ++i) {
        raw_data[i] = (raw_data[i] << 1) | ((raw_data[i + 1] >> 7) & 1);
    }
    raw_data[7] = (raw_data[7] << 1) | data;

    if(ioprox.can_be_decoded(raw_data, sizeof(raw_data))) {
        ready = true;
    }
}

void DecoderIoProx::reset_state() {
    current_demodulated_value = false;
    demodulated_value_invalid = false;

    current_period_duration = 0;
    demodulation_sample_duration = 0;

    ready = false;
}
