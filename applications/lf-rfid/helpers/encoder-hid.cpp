#include "encoder-hid.h"
#include <furi.h>

void EncoderHID::init(const uint8_t* data, const uint8_t data_size) {
    card_data = 0b1010000000000000000000000000000010011101111110011001001001010010;
    card_data_index = 0;
}

void EncoderHID::get_next(bool* polarity, uint16_t* period, uint16_t* pulse) {
    *period = 100;
    *pulse = 50;
    *polarity = true;
}
