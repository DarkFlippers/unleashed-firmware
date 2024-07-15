#include "felica_crc.h"

#include <furi/furi.h>

#define FELICA_CRC_POLY (0x1021U) // Polynomial: x^16 + x^12 + x^5 + 1
#define FELICA_CRC_INIT (0x0000U)

uint16_t felica_crc_calculate(const uint8_t* data, size_t length) {
    furi_check(data);

    uint16_t crc = FELICA_CRC_INIT;

    for(size_t i = 0; i < length; i++) {
        crc ^= ((uint16_t)data[i] << 8);
        for(size_t j = 0; j < 8; j++) {
            if(crc & 0x8000) {
                crc <<= 1;
                crc ^= FELICA_CRC_POLY;
            } else {
                crc <<= 1;
            }
        }
    }

    return (crc << 8) | (crc >> 8);
}

void felica_crc_append(BitBuffer* buf) {
    furi_check(buf);
    const uint8_t* data = bit_buffer_get_data(buf);
    const size_t data_size = bit_buffer_get_size_bytes(buf);

    const uint16_t crc = felica_crc_calculate(data, data_size);
    bit_buffer_append_bytes(buf, (const uint8_t*)&crc, FELICA_CRC_SIZE);
}

bool felica_crc_check(const BitBuffer* buf) {
    furi_check(buf);
    const size_t data_size = bit_buffer_get_size_bytes(buf);
    if(data_size <= FELICA_CRC_SIZE) return false;

    uint16_t crc_received;
    bit_buffer_write_bytes_mid(buf, &crc_received, data_size - FELICA_CRC_SIZE, FELICA_CRC_SIZE);

    const uint8_t* data = bit_buffer_get_data(buf);
    const uint16_t crc_calc = felica_crc_calculate(data, data_size - FELICA_CRC_SIZE);

    return crc_calc == crc_received;
}

void felica_crc_trim(BitBuffer* buf) {
    furi_check(buf);
    const size_t data_size = bit_buffer_get_size_bytes(buf);
    furi_assert(data_size > FELICA_CRC_SIZE);

    bit_buffer_set_size_bytes(buf, data_size - FELICA_CRC_SIZE);
}
