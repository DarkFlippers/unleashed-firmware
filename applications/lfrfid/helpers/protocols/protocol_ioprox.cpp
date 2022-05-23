#include "protocol_ioprox.h"
#include <furi.h>
#include <cli/cli.h>

/**
 * Writes a bit into the output buffer.
 */
static void write_bit(bool bit, uint8_t position, uint8_t* data) {
    if(bit) {
        data[position / 8] |= 1UL << (7 - (position % 8));
    } else {
        data[position / 8] &= ~(1UL << (7 - (position % 8)));
    }
}

/**
 * Writes up to eight contiguous bits into the output buffer.
 */
static void write_bits(uint8_t byte, uint8_t position, uint8_t* data, uint8_t length) {
    furi_check(length <= 8);
    furi_check(length > 0);

    for(uint8_t i = 0; i < length; ++i) {
        uint8_t shift = 7 - i;
        write_bit((byte >> shift) & 1, position + i, data);
    }
}

uint8_t ProtocolIoProx::get_encoded_data_size() {
    return 8;
}

uint8_t ProtocolIoProx::get_decoded_data_size() {
    return 4;
}

void ProtocolIoProx::encode(
    const uint8_t* decoded_data,
    const uint8_t decoded_data_size,
    uint8_t* encoded_data,
    const uint8_t encoded_data_size) {
    furi_check(decoded_data_size >= get_decoded_data_size());
    furi_check(encoded_data_size >= get_encoded_data_size());

    // Packet to transmit:
    //
    // 0           10          20          30          40          50          60
    // v           v           v           v           v           v           v
    // 01234567 8 90123456 7 89012345 6 78901234 5 67890123 4 56789012 3 45678901 23
    // -----------------------------------------------------------------------------
    // 00000000 0 11110000 1 facility 1 version_ 1 code-one 1 code-two 1 checksum 11

    // Preamble.
    write_bits(0b00000000, 0, encoded_data, 8);
    write_bit(0, 8, encoded_data);

    write_bits(0b11110000, 9, encoded_data, 8);
    write_bit(1, 17, encoded_data);

    // Facility code.
    write_bits(decoded_data[0], 18, encoded_data, 8);
    write_bit(1, 26, encoded_data);

    // Version
    write_bits(decoded_data[1], 27, encoded_data, 8);
    write_bit(1, 35, encoded_data);

    // Code one
    write_bits(decoded_data[2], 36, encoded_data, 8);
    write_bit(1, 44, encoded_data);

    // Code two
    write_bits(decoded_data[3], 45, encoded_data, 8);
    write_bit(1, 53, encoded_data);

    // Checksum
    write_bits(compute_checksum(encoded_data, 8), 54, encoded_data, 8);
    write_bit(1, 62, encoded_data);
    write_bit(1, 63, encoded_data);
}

void ProtocolIoProx::decode(
    const uint8_t* encoded_data,
    const uint8_t encoded_data_size,
    uint8_t* decoded_data,
    const uint8_t decoded_data_size) {
    furi_check(decoded_data_size >= get_decoded_data_size());
    furi_check(encoded_data_size >= get_encoded_data_size());

    // Packet structure:
    // (Note: the second word seems fixed; but this may not be a guarantee;
    //  it currently has no meaning.)
    //
    //0        1        2        3        4        5        6        7
    //v        v        v        v        v        v        v        v
    //01234567 89ABCDEF 01234567 89ABCDEF 01234567 89ABCDEF 01234567 89ABCDEF
    //-----------------------------------------------------------------------
    //00000000 01111000 01FFFFFF FF1VVVVV VVV1CCCC CCCC1CCC CCCCC1XX XXXXXX11
    //
    // F = facility code
    // V = version
    // C = code
    // X = checksum

    // Facility code
    decoded_data[0] = (encoded_data[2] << 2) | (encoded_data[3] >> 6);

    // Version code.
    decoded_data[1] = (encoded_data[3] << 3) | (encoded_data[4] >> 5);

    // Code bytes.
    decoded_data[2] = (encoded_data[4] << 4) | (encoded_data[5] >> 4);
    decoded_data[3] = (encoded_data[5] << 5) | (encoded_data[6] >> 3);
}

bool ProtocolIoProx::can_be_decoded(const uint8_t* encoded_data, const uint8_t encoded_data_size) {
    furi_check(encoded_data_size >= get_encoded_data_size());

    // Packet framing
    //
    //0        1        2        3        4        5        6        7
    //v        v        v        v        v        v        v        v
    //01234567 89ABCDEF 01234567 89ABCDEF 01234567 89ABCDEF 01234567 89ABCDEF
    //-----------------------------------------------------------------------
    //00000000 01______ _1______ __1_____ ___1____ ____1___ _____1XX XXXXXX11
    //
    // _ = variable data
    // 0 = preamble 0
    // 1 = framing 1
    // X = checksum

    // Validate the packet preamble is there...
    if(encoded_data[0] != 0b00000000) {
        return false;
    }
    if((encoded_data[1] >> 6) != 0b01) {
        return false;
    }

    // ... check for known ones...
    if((encoded_data[2] & 0b01000000) == 0) {
        return false;
    }
    if((encoded_data[3] & 0b00100000) == 0) {
        return false;
    }
    if((encoded_data[4] & 0b00010000) == 0) {
        return false;
    }
    if((encoded_data[5] & 0b00001000) == 0) {
        return false;
    }
    if((encoded_data[6] & 0b00000100) == 0) {
        return false;
    }
    if((encoded_data[7] & 0b00000011) == 0) {
        return false;
    }

    // ... and validate our checksums.
    uint8_t checksum = compute_checksum(encoded_data, 8);
    uint8_t checkval = (encoded_data[6] << 6) | (encoded_data[7] >> 2);

    if(checksum != checkval) {
        return false;
    }

    return true;
}

uint8_t ProtocolIoProx::compute_checksum(const uint8_t* data, const uint8_t data_size) {
    furi_check(data_size == get_encoded_data_size());

    // Packet structure:
    //
    //0        1        2         3         4         5         6         7
    //v        v        v         v         v         v         v         v
    //01234567 8 9ABCDEF0 1 23456789 A BCDEF012 3 456789AB C DEF01234 5 6789ABCD EF
    //00000000 0 VVVVVVVV 1 WWWWWWWW 1 XXXXXXXX 1 YYYYYYYY 1 ZZZZZZZZ 1 CHECKSUM 11
    //
    // algorithm as observed by the proxmark3 folks
    // CHECKSUM == 0xFF - (V + W + X + Y + Z)

    uint8_t checksum = 0;

    checksum += (data[1] << 1) | (data[2] >> 7); // VVVVVVVVV
    checksum += (data[2] << 2) | (data[3] >> 6); // WWWWWWWWW
    checksum += (data[3] << 3) | (data[4] >> 5); // XXXXXXXXX
    checksum += (data[4] << 4) | (data[5] >> 4); // YYYYYYYYY
    checksum += (data[5] << 5) | (data[6] >> 3); // ZZZZZZZZZ

    return 0xFF - checksum;
}
