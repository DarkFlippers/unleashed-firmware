#include "encoder-hid-h10301.h"
#include <furi.h>

void EncoderHID_H10301::init(const uint8_t* data, const uint8_t data_size) {
    furi_check(data_size == 3);

    card_data[0] = 0;
    card_data[1] = 0;
    card_data[2] = 0;

    uint32_t fc_cn = (data[0] << 16) | (data[1] << 8) | data[2];

    // even parity sum calculation (high 12 bits of data)
    uint8_t even_parity_sum = 0;
    for(int8_t i = 12; i < 24; i++) {
        if(((fc_cn >> i) & 1) == 1) {
            even_parity_sum++;
        }
    }

    // odd parity sum calculation (low 12 bits of data)
    uint8_t odd_parity_sum = 1;
    for(int8_t i = 0; i < 12; i++) {
        if(((fc_cn >> i) & 1) == 1) {
            odd_parity_sum++;
        }
    }

    // 0x1D preamble
    write_raw_bit(0, 0);
    write_raw_bit(0, 1);
    write_raw_bit(0, 2);
    write_raw_bit(1, 3);
    write_raw_bit(1, 4);
    write_raw_bit(1, 5);
    write_raw_bit(0, 6);
    write_raw_bit(1, 7);

    // company / OEM code 1
    write_bit(0, 8);
    write_bit(0, 10);
    write_bit(0, 12);
    write_bit(0, 14);
    write_bit(0, 16);
    write_bit(0, 18);
    write_bit(1, 20);

    // card format / length 1
    write_bit(0, 22);
    write_bit(0, 24);
    write_bit(0, 26);
    write_bit(0, 28);
    write_bit(0, 30);
    write_bit(0, 32);
    write_bit(0, 34);
    write_bit(0, 36);
    write_bit(0, 38);
    write_bit(0, 40);
    write_bit(1, 42);

    // even parity bit
    write_bit((even_parity_sum % 2), 44);

    // data
    for(uint8_t i = 0; i < 24; i++) {
        write_bit((fc_cn >> (23 - i)) & 1, 46 + (i * 2));
    }

    // odd parity bit
    write_bit((odd_parity_sum % 2), 94);

    card_data_index = 0;
    bit_index = 0;
}

void EncoderHID_H10301::write_bit(bool bit, uint8_t position) {
    write_raw_bit(bit, position + 0);
    write_raw_bit(!bit, position + 1);
}

void EncoderHID_H10301::write_raw_bit(bool bit, uint8_t position) {
    if(bit) {
        card_data[position / 32] |= 1UL << (31 - (position % 32));
    } else {
        card_data[position / 32] &= ~(1UL << (31 - (position % 32)));
    }
}

void EncoderHID_H10301::get_next(bool* polarity, uint16_t* period, uint16_t* pulse) {
    // hid 0 is 6 cycles by 8 clocks
    const uint8_t hid_0_period = 8;
    const uint8_t hid_0_count = 6;
    // hid 1 is 5 cycles by 10 clocks
    const uint8_t hid_1_period = 10;
    const uint8_t hid_1_count = 5;

    bool bit = (card_data[card_data_index / 32] >> (31 - (card_data_index % 32))) & 1;

    *polarity = true;
    if(bit) {
        *period = hid_1_period;
        *pulse = hid_1_period / 2;

        bit_index++;
        if(bit_index >= hid_1_count) {
            bit_index = 0;
            card_data_index++;
            if(card_data_index >= (32 * card_data_max)) {
                card_data_index = 0;
            }
        }
    } else {
        *period = hid_0_period;
        *pulse = hid_0_period / 2;

        bit_index++;
        if(bit_index >= hid_0_count) {
            bit_index = 0;
            card_data_index++;
            if(card_data_index >= (32 * card_data_max)) {
                card_data_index = 0;
            }
        }
    }
}
