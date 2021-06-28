#include "protocol-indala-40134.h"
#include <furi.h>

typedef uint64_t Indala40134CardData;

static void set_bit(bool bit, uint8_t position, Indala40134CardData* card_data) {
    position = (sizeof(Indala40134CardData) * 8) - 1 - position;
    if(bit) {
        *card_data |= 1ull << position;
    } else {
        *card_data &= ~(1ull << position);
    }
}

uint8_t ProtocolIndala40134::get_encoded_data_size() {
    return sizeof(Indala40134CardData);
}

uint8_t ProtocolIndala40134::get_decoded_data_size() {
    return 3;
}

void ProtocolIndala40134::encode(
    const uint8_t* decoded_data,
    const uint8_t decoded_data_size,
    uint8_t* encoded_data,
    const uint8_t encoded_data_size) {
    furi_check(decoded_data_size >= get_decoded_data_size());
    furi_check(encoded_data_size >= get_encoded_data_size());

    uint32_t fc_and_card = (decoded_data[0] << 16) | (decoded_data[1] << 8) | decoded_data[2];
    Indala40134CardData card_data = 0;

    // preamble
    set_bit(1, 0, &card_data);
    set_bit(1, 2, &card_data);
    set_bit(1, 32, &card_data);

    // factory code
    set_bit(((fc_and_card >> 23) & 1), 57, &card_data);
    set_bit(((fc_and_card >> 22) & 1), 49, &card_data);
    set_bit(((fc_and_card >> 21) & 1), 44, &card_data);
    set_bit(((fc_and_card >> 20) & 1), 47, &card_data);
    set_bit(((fc_and_card >> 19) & 1), 48, &card_data);
    set_bit(((fc_and_card >> 18) & 1), 53, &card_data);
    set_bit(((fc_and_card >> 17) & 1), 39, &card_data);
    set_bit(((fc_and_card >> 16) & 1), 58, &card_data);

    // card number
    set_bit(((fc_and_card >> 15) & 1), 42, &card_data);
    set_bit(((fc_and_card >> 14) & 1), 45, &card_data);
    set_bit(((fc_and_card >> 13) & 1), 43, &card_data);
    set_bit(((fc_and_card >> 12) & 1), 40, &card_data);
    set_bit(((fc_and_card >> 11) & 1), 52, &card_data);
    set_bit(((fc_and_card >> 10) & 1), 36, &card_data);
    set_bit(((fc_and_card >> 9) & 1), 35, &card_data);
    set_bit(((fc_and_card >> 8) & 1), 51, &card_data);
    set_bit(((fc_and_card >> 7) & 1), 46, &card_data);
    set_bit(((fc_and_card >> 6) & 1), 33, &card_data);
    set_bit(((fc_and_card >> 5) & 1), 37, &card_data);
    set_bit(((fc_and_card >> 4) & 1), 54, &card_data);
    set_bit(((fc_and_card >> 3) & 1), 56, &card_data);
    set_bit(((fc_and_card >> 2) & 1), 59, &card_data);
    set_bit(((fc_and_card >> 1) & 1), 50, &card_data);
    set_bit(((fc_and_card >> 0) & 1), 41, &card_data);

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
    set_bit((even_parity_sum % 2), 34, &card_data);

    // odd parity bit
    set_bit((odd_parity_sum % 2), 38, &card_data);

    // checksum
    if((checksum & 1) == 1) {
        set_bit(0, 62, &card_data);
        set_bit(1, 63, &card_data);
    } else {
        set_bit(1, 62, &card_data);
        set_bit(0, 63, &card_data);
    }

    memcpy(encoded_data, &card_data, get_encoded_data_size());
}

void ProtocolIndala40134::decode(
    const uint8_t* encoded_data,
    const uint8_t encoded_data_size,
    uint8_t* decoded_data,
    const uint8_t decoded_data_size) {
    furi_check(decoded_data_size >= get_decoded_data_size());
    furi_check(encoded_data_size >= get_encoded_data_size());
    // TODO implement decoding
    furi_check(0);
}

bool ProtocolIndala40134::can_be_decoded(
    const uint8_t* encoded_data,
    const uint8_t encoded_data_size) {
    furi_check(encoded_data_size >= get_encoded_data_size());
    // TODO implement decoding
    furi_check(0);
    return false;
}