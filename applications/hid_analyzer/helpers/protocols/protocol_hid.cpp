#include "protocol_hid.h"
#include <furi.h>

typedef uint32_t HIDCardData;
constexpr uint8_t HIDCount = 3;
constexpr uint8_t HIDBitSize = sizeof(HIDCardData) * 8;

uint8_t ProtocolHID::get_encoded_data_size() {
    return sizeof(HIDCardData) * HIDCount;
}

uint8_t ProtocolHID::get_decoded_data_size() {
    return 3;
}

void ProtocolHID::encode(
    const uint8_t* decoded_data,
    const uint8_t decoded_data_size,
    uint8_t* encoded_data,
    const uint8_t encoded_data_size) {
    UNUSED(decoded_data);
    UNUSED(decoded_data_size);
    UNUSED(encoded_data);
    UNUSED(encoded_data_size);
    // bob!
}

void ProtocolHID::decode(
    const uint8_t* encoded_data,
    const uint8_t encoded_data_size,
    uint8_t* decoded_data,
    const uint8_t decoded_data_size) {
    furi_check(decoded_data_size >= get_decoded_data_size());
    furi_check(encoded_data_size >= get_encoded_data_size());

    // header check
    int16_t second1pos = find_second_1(encoded_data);

    if((*(encoded_data + 1) & 0b1100) != 0x08) {
        *decoded_data = 37;
    } else {
        *decoded_data = (36 - (second1pos - 8));
    }
}

int16_t ProtocolHID::find_second_1(const uint8_t* encoded_data) {
    if((*(encoded_data + 1) & 0b11) == 0b10) {
        return 8;
    } else {
        for(int8_t i = 3; i >= 0; i--) {
            if(((*(encoded_data + 0) >> (2 * i)) & 0b11) == 0b10) {
                return (12 - i);
            }
        }
        for(int8_t i = 3; i >= 0; i--) {
            if(((*(encoded_data + 7) >> (2 * i)) & 0b11) == 0b10) {
                return (16 - i);
            }
        }
        for(int8_t i = 3; i >= 2; i--) {
            if(((*(encoded_data + 6) >> (2 * i)) & 0b11) == 0b10) {
                return (20 - i);
            }
        }
    }

    return -1;
}

bool ProtocolHID::can_be_decoded(const uint8_t* encoded_data, const uint8_t encoded_data_size) {
    furi_check(encoded_data_size >= get_encoded_data_size());

    const HIDCardData* card_data = reinterpret_cast<const HIDCardData*>(encoded_data);

    // header check
    int16_t second1pos = -1;
    // packet pre-preamble
    if(*(encoded_data + 3) != 0x1D) {
        return false;
    }

    // packet preamble
    if(*(encoded_data + 2) != 0x55) { // first four 0s mandatory in preamble
        return false;
    }

    if((*(encoded_data + 1) & 0xF0) != 0x50) { // next two 0s mandatory in preamble
        return false;
    }

    if((*(encoded_data + 1) & 0b1100) != 0x08) { // if it's not a 1...
        // either it's a 37-bit or invalid
        // so just continue with the manchester encoding checks
    } else { // it is a 1. so it could be anywhere between 26 and 36 bit encoding. or invalid.
        // we need to find the location of the second 1
        second1pos = find_second_1(encoded_data);
    }

    if(second1pos == -1) {
        // we're 37 bit or invalid
    }

    // data decoding. ensure all is properly manchester encoded
    uint32_t result = 0;

    // decode from word 0
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 11; i >= 0; i--) {
        switch((*(card_data + 0) >> (2 * i)) & 0b11) {
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

    // decode from word 1
    // coded with 01 = 0, 10 = 1 transitions
    for(int8_t i = 15; i >= 0; i--) {
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

    return true;
}
