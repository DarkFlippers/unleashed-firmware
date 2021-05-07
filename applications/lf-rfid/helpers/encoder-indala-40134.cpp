#include "encoder-indala-40134.h"
#include <furi.h>

void EncoderIndala_40134::init(const uint8_t* data, const uint8_t data_size) {
    furi_check(data_size == 3);
    uint32_t fc_and_card = (data[0] << 16) | (data[1] << 8) | data[2];

    card_data = 0;

    // preamble
    set_bit(1, 0);
    set_bit(1, 2);
    set_bit(1, 32);

    // factory code
    set_bit(((fc_and_card >> 23) & 1), 57);
    set_bit(((fc_and_card >> 22) & 1), 49);
    set_bit(((fc_and_card >> 21) & 1), 44);
    set_bit(((fc_and_card >> 20) & 1), 47);
    set_bit(((fc_and_card >> 19) & 1), 48);
    set_bit(((fc_and_card >> 18) & 1), 53);
    set_bit(((fc_and_card >> 17) & 1), 39);
    set_bit(((fc_and_card >> 16) & 1), 58);

    // card number
    set_bit(((fc_and_card >> 15) & 1), 42);
    set_bit(((fc_and_card >> 14) & 1), 45);
    set_bit(((fc_and_card >> 13) & 1), 43);
    set_bit(((fc_and_card >> 12) & 1), 40);
    set_bit(((fc_and_card >> 11) & 1), 52);
    set_bit(((fc_and_card >> 10) & 1), 36);
    set_bit(((fc_and_card >> 9) & 1), 35);
    set_bit(((fc_and_card >> 8) & 1), 51);
    set_bit(((fc_and_card >> 7) & 1), 46);
    set_bit(((fc_and_card >> 6) & 1), 33);
    set_bit(((fc_and_card >> 5) & 1), 37);
    set_bit(((fc_and_card >> 4) & 1), 54);
    set_bit(((fc_and_card >> 3) & 1), 56);
    set_bit(((fc_and_card >> 2) & 1), 59);
    set_bit(((fc_and_card >> 1) & 1), 50);
    set_bit(((fc_and_card >> 0) & 1), 41);

    // checksum
    uint8_t checksum = 0;
    checksum += ((fc_and_card >> 14) & 1);
    checksum += ((fc_and_card >> 12) & 1);
    checksum += ((fc_and_card >> 9) & 1);
    checksum += ((fc_and_card >> 8) & 1);
    checksum += ((fc_and_card >> 6) & 1);
    checksum += ((fc_and_card >> 5) & 1);
    checksum += ((fc_and_card >> 2) & 1);
    checksum += ((fc_and_card >> 0) & 1);

    // wiegand parity bits
    // even parity sum calculation (high 12 bits of data)
    uint8_t even_parity_sum = 0;
    for(int8_t i = 12; i < 24; i++) {
        if(((fc_and_card >> i) & 1) == 1) {
            even_parity_sum++;
        }
    }

    // odd parity sum calculation (low 12 bits of data)
    uint8_t odd_parity_sum = 1;
    for(int8_t i = 0; i < 12; i++) {
        if(((fc_and_card >> i) & 1) == 1) {
            odd_parity_sum++;
        }
    }

    // even parity bit
    set_bit((even_parity_sum % 2), 34);

    // odd parity bit
    set_bit((odd_parity_sum % 2), 38);

    // checksum
    if((checksum & 1) == 1) {
        set_bit(0, 62);
        set_bit(1, 63);
    } else {
        set_bit(1, 62);
        set_bit(0, 63);
    }

    last_bit = card_data & 1;
    card_data_index = 0;
    current_polarity = true;
}

void EncoderIndala_40134::set_bit(bool bit, uint8_t position) {
    position = 63 - position;
    if(bit) {
        card_data |= 1ull << position;
    } else {
        card_data &= ~(1ull << position);
    }
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
