#include "encoder-emmarine.h"
#include <furi.h>

void EncoderEM::init(const uint8_t* data, const uint8_t data_size) {
    furi_check(data_size == 5);

    // header
    card_data = 0b111111111;

    // data
    for(uint8_t i = 0; i < 5; i++) {
        write_nibble(false, data[i]);
        write_nibble(true, data[i]);
    }

    // column parity and stop bit
    uint8_t parity_sum;

    for(uint8_t c = 0; c < 4; c++) {
        parity_sum = 0;
        for(uint8_t i = 1; i <= 10; i++) {
            uint8_t parity_bit = (card_data >> (i * 5 - 1)) & 1;
            parity_sum += parity_bit;
        }
        card_data = (card_data << 1) | ((parity_sum % 2) & 1);
    }

    // stop bit
    card_data = (card_data << 1) | 0;
    card_data_index = 0;
}

void EncoderEM::write_nibble(bool low_nibble, uint8_t data) {
    uint8_t parity_sum = 0;
    uint8_t start = 0;
    if(!low_nibble) start = 4;

    for(int8_t i = (start + 3); i >= start; i--) {
        parity_sum += (data >> i) & 1;
        card_data = (card_data << 1) | ((data >> i) & 1);
    }

    card_data = (card_data << 1) | ((parity_sum % 2) & 1);
}

// data transmitted as manchester encoding
// 0 - high2low
// 1 - low2high
void EncoderEM::get_next(bool* polarity, uint16_t* period, uint16_t* pulse) {
    *period = clocks_per_bit;
    *pulse = clocks_per_bit / 2;
    *polarity = (card_data >> (63 - card_data_index)) & 1;

    card_data_index++;
    if(card_data_index >= 64) {
        card_data_index = 0;
    }
}
