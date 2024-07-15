#include "iso14443_crc.h"

#include <core/check.h>

#define ISO14443_3A_CRC_INIT (0x6363U)
#define ISO14443_3B_CRC_INIT (0xFFFFU)

static uint16_t
    iso14443_crc_calculate(Iso14443CrcType type, const uint8_t* data, size_t data_size) {
    uint16_t crc;

    if(type == Iso14443CrcTypeA) {
        crc = ISO14443_3A_CRC_INIT;
    } else if(type == Iso14443CrcTypeB) {
        crc = ISO14443_3B_CRC_INIT;
    } else {
        furi_crash("Wrong ISO14443 CRC type");
    }

    for(size_t i = 0; i < data_size; i++) {
        uint8_t byte = data[i];
        byte ^= (uint8_t)(crc & 0xff);
        byte ^= byte << 4;
        crc = (crc >> 8) ^ (((uint16_t)byte) << 8) ^ (((uint16_t)byte) << 3) ^ (byte >> 4);
    }

    return type == Iso14443CrcTypeA ? crc : ~crc;
}

void iso14443_crc_append(Iso14443CrcType type, BitBuffer* buf) {
    furi_check(buf);

    const uint8_t* data = bit_buffer_get_data(buf);
    const size_t data_size = bit_buffer_get_size_bytes(buf);

    const uint16_t crc = iso14443_crc_calculate(type, data, data_size);
    bit_buffer_append_bytes(buf, (const uint8_t*)&crc, ISO14443_CRC_SIZE);
}

bool iso14443_crc_check(Iso14443CrcType type, const BitBuffer* buf) {
    furi_check(buf);

    const size_t data_size = bit_buffer_get_size_bytes(buf);
    if(data_size <= ISO14443_CRC_SIZE) return false;

    uint16_t crc_received;
    bit_buffer_write_bytes_mid(
        buf, &crc_received, data_size - ISO14443_CRC_SIZE, ISO14443_CRC_SIZE);

    const uint8_t* data = bit_buffer_get_data(buf);
    const uint16_t crc_calc = iso14443_crc_calculate(type, data, data_size - ISO14443_CRC_SIZE);

    return crc_calc == crc_received;
}

void iso14443_crc_trim(BitBuffer* buf) {
    furi_check(buf);
    const size_t data_size = bit_buffer_get_size_bytes(buf);
    furi_check(data_size > ISO14443_CRC_SIZE);

    bit_buffer_set_size_bytes(buf, data_size - ISO14443_CRC_SIZE);
}
