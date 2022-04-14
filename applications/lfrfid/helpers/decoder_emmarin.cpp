#include "emmarin.h"
#include "decoder_emmarin.h"
#include <furi.h>
#include <furi_hal.h>

constexpr uint32_t clocks_in_us = 64;
constexpr uint32_t short_time = 255 * clocks_in_us;
constexpr uint32_t long_time = 510 * clocks_in_us;
constexpr uint32_t jitter_time = 100 * clocks_in_us;

constexpr uint32_t short_time_low = short_time - jitter_time;
constexpr uint32_t short_time_high = short_time + jitter_time;
constexpr uint32_t long_time_low = long_time - jitter_time;
constexpr uint32_t long_time_high = long_time + jitter_time;

void DecoderEMMarin::reset_state() {
    ready = false;
    read_data = 0;
    manchester_advance(
        manchester_saved_state, ManchesterEventReset, &manchester_saved_state, nullptr);
}

bool DecoderEMMarin::read(uint8_t* data, uint8_t data_size) {
    bool result = false;

    if(ready) {
        result = true;
        em_marin.decode(
            reinterpret_cast<const uint8_t*>(&read_data), sizeof(uint64_t), data, data_size);
        ready = false;
    }

    return result;
}

void DecoderEMMarin::process_front(bool polarity, uint32_t time) {
    if(ready) return;
    if(time < short_time_low) return;

    ManchesterEvent event = ManchesterEventReset;

    if(time > short_time_low && time < short_time_high) {
        if(polarity) {
            event = ManchesterEventShortHigh;
        } else {
            event = ManchesterEventShortLow;
        }
    } else if(time > long_time_low && time < long_time_high) {
        if(polarity) {
            event = ManchesterEventLongHigh;
        } else {
            event = ManchesterEventLongLow;
        }
    }

    if(event != ManchesterEventReset) {
        bool data;
        bool data_ok =
            manchester_advance(manchester_saved_state, event, &manchester_saved_state, &data);

        if(data_ok) {
            read_data = (read_data << 1) | data;

            ready = em_marin.can_be_decoded(
                reinterpret_cast<const uint8_t*>(&read_data), sizeof(uint64_t));
        }
    }
}

DecoderEMMarin::DecoderEMMarin() {
    reset_state();
}
