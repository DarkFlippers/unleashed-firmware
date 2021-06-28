#include "encoder-hid-h10301.h"
#include "protocols/protocol-hid-h10301.h"
#include <furi.h>

void EncoderHID_H10301::init(const uint8_t* data, const uint8_t data_size) {
    ProtocolHID10301 hid;
    hid.encode(data, data_size, reinterpret_cast<uint8_t*>(&card_data), sizeof(card_data) * 3);

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
