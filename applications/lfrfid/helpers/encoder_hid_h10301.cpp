#include "encoder_hid_h10301.h"
#include "protocols/protocol_hid_h10301.h"
#include <furi.h>

void EncoderHID_H10301::init(const uint8_t* data, const uint8_t data_size) {
    ProtocolHID10301 hid;
    hid.encode(data, data_size, reinterpret_cast<uint8_t*>(&card_data), sizeof(card_data) * 3);

    card_data_index = 0;
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
    uint8_t bit = (card_data[card_data_index / 32] >> (31 - (card_data_index % 32))) & 1;

    bool advance = fsk->next(bit, period);
    if(advance) {
        card_data_index++;
        if(card_data_index >= (32 * card_data_max)) {
            card_data_index = 0;
        }
    }

    *polarity = true;
    *pulse = *period / 2;
}

EncoderHID_H10301::EncoderHID_H10301() {
    fsk = new OscFSK(8, 10, 50);
}

EncoderHID_H10301::~EncoderHID_H10301() {
    delete fsk;
}
