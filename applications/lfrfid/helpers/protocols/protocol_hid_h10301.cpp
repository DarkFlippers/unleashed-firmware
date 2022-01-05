#include "protocol_hid_h10301.h"
#include <furi.h>

typedef uint32_t HID10301CardData;
constexpr uint8_t HID10301Count = 3;
constexpr uint8_t HID10301BitSize = sizeof(HID10301CardData) * 8;

static void write_raw_bit(bool bit, uint8_t position, HID10301CardData* card_data) {
    if(bit) {
        card_data[position / HID10301BitSize] |=
            1UL << (HID10301BitSize - (position % HID10301BitSize) - 1);
    } else {
        card_data[position / (sizeof(HID10301CardData) * 8)] &=
            ~(1UL << (HID10301BitSize - (position % HID10301BitSize) - 1));
    }
}

static void write_bit(bool bit, uint8_t position, HID10301CardData* card_data) {
    write_raw_bit(bit, position + 0, card_data);
    write_raw_bit(!bit, position + 1, card_data);
}

uint8_t ProtocolHID10301::get_encoded_data_size() {
    return sizeof(HID10301CardData) * HID10301Count;
}

uint8_t ProtocolHID10301::get_decoded_data_size() {
    return 3;
}

void ProtocolHID10301::encode(
    const uint8_t* decoded_data,
    const uint8_t decoded_data_size,
    uint8_t* encoded_data,
    const uint8_t encoded_data_size) {
    furi_check(decoded_data_size >= get_decoded_data_size());
    furi_check(encoded_data_size >= get_encoded_data_size());

    HID10301CardData card_data[HID10301Count] = {0, 0, 0};

    uint32_t fc_cn = (decoded_data[0] << 16) | (decoded_data[1] << 8) | decoded_data[2];

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
    write_raw_bit(0, 0, card_data);
    write_raw_bit(0, 1, card_data);
    write_raw_bit(0, 2, card_data);
    write_raw_bit(1, 3, card_data);
    write_raw_bit(1, 4, card_data);
    write_raw_bit(1, 5, card_data);
    write_raw_bit(0, 6, card_data);
    write_raw_bit(1, 7, card_data);

    // company / OEM code 1
    write_bit(0, 8, card_data);
    write_bit(0, 10, card_data);
    write_bit(0, 12, card_data);
    write_bit(0, 14, card_data);
    write_bit(0, 16, card_data);
    write_bit(0, 18, card_data);
    write_bit(1, 20, card_data);

    // card format / length 1
    write_bit(0, 22, card_data);
    write_bit(0, 24, card_data);
    write_bit(0, 26, card_data);
    write_bit(0, 28, card_data);
    write_bit(0, 30, card_data);
    write_bit(0, 32, card_data);
    write_bit(0, 34, card_data);
    write_bit(0, 36, card_data);
    write_bit(0, 38, card_data);
    write_bit(0, 40, card_data);
    write_bit(1, 42, card_data);

    // even parity bit
    write_bit((even_parity_sum % 2), 44, card_data);

    // data
    for(uint8_t i = 0; i < 24; i++) {
        write_bit((fc_cn >> (23 - i)) & 1, 46 + (i * 2), card_data);
    }

    // odd parity bit
    write_bit((odd_parity_sum % 2), 94, card_data);

    memcpy(encoded_data, &card_data, get_encoded_data_size());
}

void ProtocolHID10301::decode(
    const uint8_t* encoded_data,
    const uint8_t encoded_data_size,
    uint8_t* decoded_data,
    const uint8_t decoded_data_size) {
    furi_check(decoded_data_size >= get_decoded_data_size());
    furi_check(encoded_data_size >= get_encoded_data_size());

    const HID10301CardData* card_data = reinterpret_cast<const HID10301CardData*>(encoded_data);

    // data decoding
    uint32_t result = 0;

    // decode from word 1
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 9; i >= 0; i--) {
        switch((*(card_data + 1) >> (2 * i)) & 0b11) {
        case 0b01:
            result = (result << 1) | 0;
            break;
        case 0b10:
            result = (result << 1) | 1;
            break;
        default:
            break;
        }
    }

    // decode from word 2
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 15; i >= 0; i--) {
        switch((*(card_data + 2) >> (2 * i)) & 0b11) {
        case 0b01:
            result = (result << 1) | 0;
            break;
        case 0b10:
            result = (result << 1) | 1;
            break;
        default:
            break;
        }
    }

    uint8_t data[3] = {(uint8_t)(result >> 17), (uint8_t)(result >> 9), (uint8_t)(result >> 1)};

    memcpy(decoded_data, &data, get_decoded_data_size());
}

bool ProtocolHID10301::can_be_decoded(const uint8_t* encoded_data, const uint8_t encoded_data_size) {
    furi_check(encoded_data_size >= get_encoded_data_size());

    const HID10301CardData* card_data = reinterpret_cast<const HID10301CardData*>(encoded_data);

    // packet preamble
    // raw data
    if(*(encoded_data + 3) != 0x1D) {
        return false;
    }

    // encoded company/oem
    // coded with 01 = 0, 10 = 1 transitions
    // stored in word 0
    if((*card_data >> 10 & 0x3FFF) != 0x1556) {
        return false;
    }

    // encoded format/length
    // coded with 01 = 0, 10 = 1 transitions
    // stored in word 0 and word 1
    if((((*card_data & 0x3FF) << 12) | ((*(card_data + 1) >> 20) & 0xFFF)) != 0x155556) {
        return false;
    }

    // data decoding
    uint32_t result = 0;

    // decode from word 1
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 9; i >= 0; i--) {
        switch((*(card_data + 1) >> (2 * i)) & 0b11) {
        case 0b01:
            result = (result << 1) | 0;
            break;
        case 0b10:
            result = (result << 1) | 1;
            break;
        default:
            return false;
            break;
        }
    }

    // decode from word 2
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 15; i >= 0; i--) {
        switch((*(card_data + 2) >> (2 * i)) & 0b11) {
        case 0b01:
            result = (result << 1) | 0;
            break;
        case 0b10:
            result = (result << 1) | 1;
            break;
        default:
            return false;
            break;
        }
    }

    // trailing parity (odd) test
    uint8_t parity_sum = 0;
    for(int8_t i = 0; i < 13; i++) {
        if(((result >> i) & 1) == 1) {
            parity_sum++;
        }
    }

    if((parity_sum % 2) != 1) {
        return false;
    }

    // leading parity (even) test
    parity_sum = 0;
    for(int8_t i = 13; i < 26; i++) {
        if(((result >> i) & 1) == 1) {
            parity_sum++;
        }
    }

    if((parity_sum % 2) == 1) {
        return false;
    }

    return true;
}
