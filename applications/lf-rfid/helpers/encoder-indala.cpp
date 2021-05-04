#include "encoder-indala.h"
#include <furi.h>

void EncoderIndala::init(const uint8_t* data, const uint8_t data_size) {
    card_data = 0b1010000000000000000000000000000010011101111110011001001001010010;
    last_polarity = card_data & 1;
    card_data_index = 0;
}

void EncoderIndala::get_next(bool* polarity, uint16_t* period, uint16_t* pulse) {
    bool new_bit = (card_data >> (63 - card_data_index)) & 1;

    *period = 2;
    *pulse = 1;
    *polarity = (new_bit != last_polarity);

    bit_clock_index++;
    if(bit_clock_index >= clock_per_bit) {
        bit_clock_index = 0;
        last_polarity = *polarity;

        card_data_index++;
        if(card_data_index >= 64) {
            card_data_index = 0;
        }
    }
}
