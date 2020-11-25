#include "maxim_crc.h"

uint8_t maxim_crc8(const uint8_t* data, const uint8_t data_size, const uint8_t crc_init) {
    uint8_t crc = crc_init;

    for(uint8_t index = 0; index < data_size; ++index) {
        uint8_t input_byte = data[index];
        for(uint8_t bit_position = 0; bit_position < 8; ++bit_position) {
            const uint8_t mix = (crc ^ input_byte) & static_cast<uint8_t>(0x01);
            crc >>= 1;
            if(mix != 0) crc ^= 0x8C;
            input_byte >>= 1;
        }
    }
    return crc;
}

uint16_t maxim_crc16(const uint8_t* address, const uint8_t length, const uint16_t init) {
    uint16_t crc = init;

    static const uint8_t odd_parity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    for(uint8_t i = 0; i < length; ++i) {
        uint16_t cdata = address[i];
        cdata = (cdata ^ crc) & static_cast<uint16_t>(0xff);
        crc >>= 8;

        if((odd_parity[cdata & 0x0F] ^ odd_parity[cdata >> 4]) != 0) crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }

    return crc;
}

uint16_t maxim_crc16(uint8_t value, uint16_t crc) {
    static const uint8_t odd_parity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    value = (value ^ static_cast<uint8_t>(crc));
    crc >>= 8;
    if((odd_parity[value & 0x0F] ^ odd_parity[value >> 4]) != 0) crc ^= 0xC001;
    uint16_t cdata = (static_cast<uint16_t>(value) << 6);
    crc ^= cdata;
    crc ^= (static_cast<uint16_t>(cdata) << 1);
    return crc;
}