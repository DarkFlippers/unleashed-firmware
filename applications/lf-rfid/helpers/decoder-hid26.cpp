#include "decoder-hid26.h"
#include <api-hal.h>

constexpr uint32_t clocks_in_us = 64;

constexpr uint32_t jitter_time_us = 20;
constexpr uint32_t min_time_us = 64;
constexpr uint32_t max_time_us = 80;

constexpr uint32_t min_time = (min_time_us - jitter_time_us) * clocks_in_us;
constexpr uint32_t mid_time = ((max_time_us - min_time_us) / 2 + min_time_us) * clocks_in_us;
constexpr uint32_t max_time = (max_time_us + jitter_time_us) * clocks_in_us;

bool DecoderHID26::read(uint8_t* data, uint8_t data_size) {
    bool result = false;
    furi_assert(data_size >= 3);

    if(ready) {
        result = true;
        data[0] = facility;
        data[1] = (uint8_t)(number >> 8);
        data[2] = (uint8_t)number;

        //printf("HID %02X %02X %02X\r\n", facility, (uint8_t)(number >> 8), (uint8_t)number);
        ready = false;
    }

    return result;
}

void DecoderHID26::process_front(bool polarity, uint32_t time) {
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

DecoderHID26::DecoderHID26() {
    reset_state();
}

void DecoderHID26::store_data(bool data) {
    stored_data[0] = (stored_data[0] << 1) | ((stored_data[1] >> 31) & 1);
    stored_data[1] = (stored_data[1] << 1) | ((stored_data[2] >> 31) & 1);
    stored_data[2] = (stored_data[2] << 1) | data;
    validate_stored_data();
}

void DecoderHID26::validate_stored_data() {
    // packet preamble
    // raw data
    if(*(reinterpret_cast<uint8_t*>(stored_data) + 3) != 0x1D) {
        return;
    }

    // encoded company/oem
    // coded with 01 = 0, 10 = 1 transitions
    // stored in word 0
    if((*stored_data >> 10 & 0x3FFF) != 0x1556) {
        return;
    }

    // encoded format/length
    // coded with 01 = 0, 10 = 1 transitions
    // stored in word 0 and word 1
    if((((*stored_data & 0x3FF) << 12) | ((*(stored_data + 1) >> 20) & 0xFFF)) != 0x155556) {
        return;
    }

    // data decoding
    uint32_t result = 0;

    // decode from word 1
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 9; i >= 0; i--) {
        switch((*(stored_data + 1) >> (2 * i)) & 0b11) {
        case 0b01:
            result = (result << 1) | 0;
            break;
        case 0b10:
            result = (result << 1) | 1;
            break;
        default:
            return;
            break;
        }
    }

    // decode from word 2
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 15; i >= 0; i--) {
        switch((*(stored_data + 2) >> (2 * i)) & 0b11) {
        case 0b01:
            result = (result << 1) | 0;
            break;
        case 0b10:
            result = (result << 1) | 1;
            break;
        default:
            return;
            break;
        }
    }

    // store decoded data
    facility = result >> 17;
    number = result >> 1;

    // trailing parity (odd) test
    uint8_t parity_sum = 0;
    for(int8_t i = 0; i < 13; i++) {
        if(((result >> i) & 1) == 1) {
            parity_sum++;
        }
    }

    if((parity_sum % 2) != 1) {
        return;
    }

    // leading parity (even) test
    parity_sum = 0;
    for(int8_t i = 13; i < 26; i++) {
        if(((result >> i) & 1) == 1) {
            parity_sum++;
        }
    }

    if((parity_sum % 2) == 1) {
        return;
    }

    ready = true;
}

void DecoderHID26::reset_state() {
    last_pulse = false;
    pulse_count = 0;
    ready = false;
    last_pulse_time = 0;
}
