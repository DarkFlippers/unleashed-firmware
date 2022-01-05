#include "protocol_emmarin.h"
#include <furi.h>

#define EM_HEADER_POS 55
#define EM_HEADER_MASK (0x1FFLLU << EM_HEADER_POS)

#define EM_FIRST_ROW_POS 50

#define EM_ROW_COUNT 10
#define EM_COLUMN_COUNT 4
#define EM_BITS_PER_ROW_COUNT (EM_COLUMN_COUNT + 1)

#define EM_COLUMN_POS 4
#define EM_STOP_POS 0
#define EM_STOP_MASK (0x1LLU << EM_STOP_POS)

#define EM_HEADER_AND_STOP_MASK (EM_HEADER_MASK | EM_STOP_MASK)
#define EM_HEADER_AND_STOP_DATA (EM_HEADER_MASK)

typedef uint64_t EMMarinCardData;

void write_nibble(bool low_nibble, uint8_t data, EMMarinCardData* card_data) {
    uint8_t parity_sum = 0;
    uint8_t start = 0;
    if(!low_nibble) start = 4;

    for(int8_t i = (start + 3); i >= start; i--) {
        parity_sum += (data >> i) & 1;
        *card_data = (*card_data << 1) | ((data >> i) & 1);
    }

    *card_data = (*card_data << 1) | ((parity_sum % 2) & 1);
}

uint8_t ProtocolEMMarin::get_encoded_data_size() {
    return sizeof(EMMarinCardData);
}

uint8_t ProtocolEMMarin::get_decoded_data_size() {
    return 5;
}

void ProtocolEMMarin::encode(
    const uint8_t* decoded_data,
    const uint8_t decoded_data_size,
    uint8_t* encoded_data,
    const uint8_t encoded_data_size) {
    furi_check(decoded_data_size >= get_decoded_data_size());
    furi_check(encoded_data_size >= get_encoded_data_size());

    EMMarinCardData card_data;

    // header
    card_data = 0b111111111;

    // data
    for(uint8_t i = 0; i < get_decoded_data_size(); i++) {
        write_nibble(false, decoded_data[i], &card_data);
        write_nibble(true, decoded_data[i], &card_data);
    }

    // column parity and stop bit
    uint8_t parity_sum;

    for(uint8_t c = 0; c < EM_COLUMN_COUNT; c++) {
        parity_sum = 0;
        for(uint8_t i = 1; i <= EM_ROW_COUNT; i++) {
            uint8_t parity_bit = (card_data >> (i * EM_BITS_PER_ROW_COUNT - 1)) & 1;
            parity_sum += parity_bit;
        }
        card_data = (card_data << 1) | ((parity_sum % 2) & 1);
    }

    // stop bit
    card_data = (card_data << 1) | 0;

    memcpy(encoded_data, &card_data, get_encoded_data_size());
}

void ProtocolEMMarin::decode(
    const uint8_t* encoded_data,
    const uint8_t encoded_data_size,
    uint8_t* decoded_data,
    const uint8_t decoded_data_size) {
    furi_check(decoded_data_size >= get_decoded_data_size());
    furi_check(encoded_data_size >= get_encoded_data_size());

    uint8_t decoded_data_index = 0;
    EMMarinCardData card_data = *(reinterpret_cast<const EMMarinCardData*>(encoded_data));

    // clean result
    memset(decoded_data, 0, decoded_data_size);

    // header
    for(uint8_t i = 0; i < 9; i++) {
        card_data = card_data << 1;
    }

    // nibbles
    uint8_t value = 0;
    for(uint8_t r = 0; r < EM_ROW_COUNT; r++) {
        uint8_t nibble = 0;
        for(uint8_t i = 0; i < 5; i++) {
            if(i < 4) nibble = (nibble << 1) | (card_data & (1LLU << 63) ? 1 : 0);
            card_data = card_data << 1;
        }
        value = (value << 4) | nibble;
        if(r % 2) {
            decoded_data[decoded_data_index] |= value;
            decoded_data_index++;
            value = 0;
        }
    }
}

bool ProtocolEMMarin::can_be_decoded(const uint8_t* encoded_data, const uint8_t encoded_data_size) {
    furi_check(encoded_data_size >= get_encoded_data_size());
    const EMMarinCardData* card_data = reinterpret_cast<const EMMarinCardData*>(encoded_data);

    // check header and stop bit
    if((*card_data & EM_HEADER_AND_STOP_MASK) != EM_HEADER_AND_STOP_DATA) return false;

    // check row parity
    for(uint8_t i = 0; i < EM_ROW_COUNT; i++) {
        uint8_t parity_sum = 0;

        for(uint8_t j = 0; j < EM_BITS_PER_ROW_COUNT; j++) {
            parity_sum += (*card_data >> (EM_FIRST_ROW_POS - i * EM_BITS_PER_ROW_COUNT + j)) & 1;
        }

        if((parity_sum % 2)) {
            return false;
        }
    }

    // check columns parity
    for(uint8_t i = 0; i < EM_COLUMN_COUNT; i++) {
        uint8_t parity_sum = 0;

        for(uint8_t j = 0; j < EM_ROW_COUNT + 1; j++) {
            parity_sum += (*card_data >> (EM_COLUMN_POS - i + j * EM_BITS_PER_ROW_COUNT)) & 1;
        }

        if((parity_sum % 2)) {
            return false;
        }
    }

    return true;
}
