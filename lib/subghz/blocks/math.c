#include "math.h"

uint64_t subghz_protocol_blocks_reverse_key(uint64_t key, uint8_t count_bit) {
    uint64_t key_reverse = 0;
    for(uint8_t i = 0; i < count_bit; i++) {
        key_reverse = key_reverse << 1 | bit_read(key, i);
    }
    return key_reverse;
}

uint8_t subghz_protocol_blocks_get_parity(uint64_t key, uint8_t count_bit) {
    uint8_t parity = 0;
    for(uint8_t i = 0; i < count_bit; i++) {
        parity += bit_read(key, i);
    }
    return parity & 0x01;
}

uint8_t subghz_protocol_blocks_crc4(
    uint8_t const message[],
    unsigned nBytes,
    uint8_t polynomial,
    uint8_t init) {
    unsigned remainder = init << 4; // LSBs are unused
    unsigned poly = polynomial << 4;
    unsigned bit;

    while(nBytes--) {
        remainder ^= *message++;
        for(bit = 0; bit < 8; bit++) {
            if(remainder & 0x80) {
                remainder = (remainder << 1) ^ poly;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder >> 4 & 0x0f; // discard the LSBs
}

uint8_t subghz_protocol_blocks_crc7(
    uint8_t const message[],
    unsigned nBytes,
    uint8_t polynomial,
    uint8_t init) {
    unsigned remainder = init << 1; // LSB is unused
    unsigned poly = polynomial << 1;
    unsigned byte, bit;

    for(byte = 0; byte < nBytes; ++byte) {
        remainder ^= message[byte];
        for(bit = 0; bit < 8; ++bit) {
            if(remainder & 0x80) {
                remainder = (remainder << 1) ^ poly;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder >> 1 & 0x7f; // discard the LSB
}

uint8_t subghz_protocol_blocks_crc8(
    uint8_t const message[],
    unsigned nBytes,
    uint8_t polynomial,
    uint8_t init) {
    uint8_t remainder = init;
    unsigned byte, bit;

    for(byte = 0; byte < nBytes; ++byte) {
        remainder ^= message[byte];
        for(bit = 0; bit < 8; ++bit) {
            if(remainder & 0x80) {
                remainder = (remainder << 1) ^ polynomial;
            } else {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder;
}