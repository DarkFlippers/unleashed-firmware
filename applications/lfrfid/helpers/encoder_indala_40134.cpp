#include "encoder_indala_40134.h"
#include "protocols/protocol_indala_40134.h"
#include <furi.h>

void EncoderIndala_40134::init(const uint8_t* data, const uint8_t data_size) {
    ProtocolIndala40134 indala;
    indala.encode(data, data_size, reinterpret_cast<uint8_t*>(&card_data), sizeof(card_data));

    last_bit = card_data & 1;
    card_data_index = 0;
    current_polarity = true;
}

void EncoderIndala_40134::get_next(bool* polarity, uint16_t* period, uint16_t* pulse) {
    *period = 2;
    *pulse = 1;
    *polarity = current_polarity;

    bit_clock_index++;
    if(bit_clock_index >= clock_per_bit) {
        bit_clock_index = 0;

        bool current_bit = (card_data >> (63 - card_data_index)) & 1;

        if(current_bit != last_bit) {
            current_polarity = !current_polarity;
        }

        last_bit = current_bit;

        card_data_index++;
        if(card_data_index >= 64) {
            card_data_index = 0;
        }
    }
}
