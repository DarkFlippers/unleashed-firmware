#include "decoder-indala.h"
#include <api-hal.h>

constexpr uint32_t clocks_in_us = 64;

constexpr uint32_t min_time_us = 25 * clocks_in_us;
constexpr uint32_t mid_time_us = 45 * clocks_in_us;
constexpr uint32_t max_time_us = 90 * clocks_in_us;

bool DecoderIndala::read(uint8_t* data, uint8_t data_size) {
    bool result = false;

    if(ready) {
        result = true;
        printf("IND %02X %02X %02X\r\n", facility, (uint8_t)(number >> 8), (uint8_t)number);
        ready = false;
    }

    return result;
}

void DecoderIndala::process_front(bool polarity, uint32_t time) {
    if(ready) return;

    if(polarity == false) {
        last_pulse_time = time;
    } else {
        last_pulse_time += time;
        pulse_count++;

        if(last_pulse_time > min_time_us && last_pulse_time < max_time_us) {
            if(last_pulse_time > mid_time_us) {
                bool last_data = !(readed_data & 1);
                pulse_count = 0;
                readed_data = (readed_data << 1) | last_data;
                verify();
            } else if((pulse_count % 16) == 0) {
                bool last_data = readed_data & 1;
                pulse_count = 0;
                readed_data = (readed_data << 1) | last_data;
                verify();
            }
        }
    }
}

DecoderIndala::DecoderIndala() {
}

void DecoderIndala::reset_state() {
}

void DecoderIndala::verify() {
    // verify inverse
    readed_data = ~readed_data;
    verify_inner();

    // verify normal
    readed_data = ~readed_data;
    verify_inner();
}

typedef union {
    uint64_t raw;
    struct __attribute__((packed)) {
        uint8_t static0 : 3;
        uint8_t checksum : 2;
        uint8_t static1 : 2;
        uint8_t y14 : 1;

        uint8_t x8 : 1;
        uint8_t x1 : 1;
        uint8_t y13 : 1;
        uint8_t static2 : 1;
        uint8_t y12 : 1;
        uint8_t x6 : 1;
        uint8_t y5 : 1;
        uint8_t y8 : 1;

        uint8_t y15 : 1;
        uint8_t x2 : 1;
        uint8_t x5 : 1;
        uint8_t x4 : 1;
        uint8_t y9 : 1;
        uint8_t y2 : 1;
        uint8_t x3 : 1;
        uint8_t y3 : 1;

        uint8_t y1 : 1;
        uint8_t y16 : 1;
        uint8_t y4 : 1;
        uint8_t x7 : 1;
        uint8_t p2 : 1;
        uint8_t y11 : 1;
        uint8_t y6 : 1;
        uint8_t y7 : 1;

        uint8_t p1 : 1;
        uint8_t y10 : 1;
        uint32_t preamble : 30;
    };
} IndalaFormat;

void DecoderIndala::verify_inner() {
    IndalaFormat id;
    id.raw = readed_data;

    // preamble
    //if((data >> 34) != 0b000000000000000000000000000001) return;
    if(id.preamble != 1) return;

    // static data bits
    //if((data & 0b100001100111) != 0b101) return;
    if(id.static2 != 0 && id.static1 != 0 && id.static0 != 0b101) return;

    // Indala checksum
    uint8_t sum_to_check = id.y2 + id.y4 + id.y7 + id.y8 + id.y10 + id.y11 + id.y14 + id.y16;

    if(sum_to_check % 2 == 0) {
        if(id.checksum != 0b10) return;
    } else {
        if(id.checksum != 0b01) return;
    }

    // read facility number
    facility = (id.x1 << 7) + (id.x2 << 6) + (id.x3 << 5) + (id.x4 << 4) + (id.x5 << 3) +
               (id.x6 << 2) + (id.x7 << 1) + (id.x8 << 0);

    // read serial number
    number = (id.y1 << 15) + (id.y2 << 14) + (id.y3 << 13) + (id.y4 << 12) + (id.y5 << 11) +
             (id.y6 << 10) + (id.y7 << 9) + (id.y8 << 8) + (id.y9 << 7) + (id.y10 << 6) +
             (id.y11 << 5) + (id.y12 << 4) + (id.y13 << 3) + (id.y14 << 2) + (id.y15 << 1) +
             (id.y16 << 0);

    // Wiegand checksum left
    sum_to_check = 0;
    for(int8_t i = 0; i < 8; i--) {
        if((facility >> i) & 1) {
            sum_to_check += 1;
        }
    }

    for(int8_t i = 0; i < 4; i--) {
        if((number >> i) & 1) {
            sum_to_check += 1;
        }
    }

    if(id.p1) {
        sum_to_check += 1;
    }

    if((sum_to_check % 2) == 1) return;

    // Wiegand checksum right
    sum_to_check = 0;
    for(int8_t i = 0; i < 12; i--) {
        if((number >> (i + 4)) & 1) {
            sum_to_check += 1;
        }
    }

    if(id.p2) {
        sum_to_check += 1;
    }

    if((sum_to_check % 2) != 1) return;

    ready = true;
}
