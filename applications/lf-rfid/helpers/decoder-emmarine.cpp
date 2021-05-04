#include "emmarine.h"
#include "decoder-emmarine.h"
#include <furi.h>
#include <api-hal.h>

constexpr uint32_t clocks_in_us = 64;
constexpr uint32_t short_time = 255 * clocks_in_us;
constexpr uint32_t long_time = 510 * clocks_in_us;
constexpr uint32_t jitter_time = 100 * clocks_in_us;

constexpr uint32_t short_time_low = short_time - jitter_time;
constexpr uint32_t short_time_high = short_time + jitter_time;
constexpr uint32_t long_time_low = long_time - jitter_time;
constexpr uint32_t long_time_high = long_time + jitter_time;

void DecoderEMMarine::reset_state() {
    ready = false;
    readed_data = 0;
    manchester_advance(
        manchester_saved_state, ManchesterEventReset, &manchester_saved_state, nullptr);
}

void printEM_raw(uint64_t data) {
    // header
    for(uint8_t i = 0; i < 9; i++) {
        printf("%u ", data & (1LLU << 63) ? 1 : 0);
        data = data << 1;
    }
    printf("\r\n");

    // nibbles
    for(uint8_t r = 0; r < 11; r++) {
        printf("        ");
        uint8_t value = 0;
        for(uint8_t i = 0; i < 5; i++) {
            printf("%u ", data & (1LLU << 63) ? 1 : 0);
            if(i < 4) value = (value << 1) | (data & (1LLU << 63) ? 1 : 0);
            data = data << 1;
        }
        printf("0x%X", value);
        printf("\r\n");
    }
}

void printEM_data(uint64_t data) {
    printf("EM ");

    // header
    for(uint8_t i = 0; i < 9; i++) {
        data = data << 1;
    }

    // nibbles
    for(uint8_t r = 0; r < EM_ROW_COUNT; r++) {
        uint8_t value = 0;
        for(uint8_t i = 0; i < 5; i++) {
            if(i < 4) value = (value << 1) | (data & (1LLU << 63) ? 1 : 0);
            data = data << 1;
        }
        printf("%X", value);
        if(r % 2) printf(" ");
    }
    printf("\r\n");
}

void copyEM_data(uint64_t data, uint8_t* result, uint8_t result_size) {
    furi_assert(result_size >= 5);
    uint8_t result_index = 0;

    // clean result
    memset(result, 0, result_size);

    // header
    for(uint8_t i = 0; i < 9; i++) {
        data = data << 1;
    }

    // nibbles
    uint8_t value = 0;
    for(uint8_t r = 0; r < EM_ROW_COUNT; r++) {
        uint8_t nibble = 0;
        for(uint8_t i = 0; i < 5; i++) {
            if(i < 4) nibble = (nibble << 1) | (data & (1LLU << 63) ? 1 : 0);
            data = data << 1;
        }
        value = (value << 4) | nibble;
        if(r % 2) {
            result[result_index] |= value;
            result_index++;
            value = 0;
        }
    }
}

bool DecoderEMMarine::read(uint8_t* data, uint8_t data_size) {
    bool result = false;

    if(ready) {
        result = true;
        copyEM_data(readed_data, data, data_size);
        ready = false;
    }

    return result;
}

void DecoderEMMarine::process_front(bool polarity, uint32_t time) {
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
            readed_data = (readed_data << 1) | data;

            // header and stop bit
            if((readed_data & EM_HEADER_AND_STOP_MASK) != EM_HEADER_AND_STOP_DATA) return;

            // row parity
            for(uint8_t i = 0; i < EM_ROW_COUNT; i++) {
                uint8_t parity_sum = 0;

                for(uint8_t j = 0; j < 5; j++) {
                    parity_sum += (readed_data >> (EM_FIRST_ROW_POS - i * 5 + j)) & 1;
                }

                if((parity_sum % 2)) {
                    return;
                }
            }

            // columns parity
            for(uint8_t i = 0; i < 4; i++) {
                uint8_t parity_sum = 0;

                for(uint8_t j = 0; j < EM_ROW_COUNT + 1; j++) {
                    parity_sum += (readed_data >> (EM_COLUMN_POS - i + j * 5)) & 1;
                }

                if((parity_sum % 2)) {
                    return;
                }
            }

            // checks ok
            ready = true;
        }
    }
}

DecoderEMMarine::DecoderEMMarine() {
    reset_state();
}
