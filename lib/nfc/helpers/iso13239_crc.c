#include "iso13239_crc.h"

#include <core/check.h>

#define ISO13239_CRC_INIT_DEFAULT  (0xFFFFU)
#define ISO13239_CRC_INIT_PICOPASS (0xE012U)
#define ISO13239_CRC_POLY          (0x8408U)

static uint16_t
    iso13239_crc_calculate(Iso13239CrcType type, const uint8_t* data, size_t data_size) {
    uint16_t crc;

    if(type == Iso13239CrcTypeDefault) {
        crc = ISO13239_CRC_INIT_DEFAULT;
    } else if(type == Iso13239CrcTypePicopass) {
        crc = ISO13239_CRC_INIT_PICOPASS;
    } else {
        furi_crash("Wrong ISO13239 CRC type");
    }

    for(size_t i = 0; i < data_size; ++i) {
        crc ^= (uint16_t)data[i];
        for(size_t j = 0; j < 8; ++j) {
            if(crc & 1U) {
                crc = (crc >> 1) ^ ISO13239_CRC_POLY;
            } else {
                crc >>= 1;
            }
        }
    }

    return type == Iso13239CrcTypePicopass ? crc : ~crc;
}

void iso13239_crc_append(Iso13239CrcType type, BitBuffer* buf) {
    furi_check(buf);

    const uint8_t* data = bit_buffer_get_data(buf);
    const size_t data_size = bit_buffer_get_size_bytes(buf);

    const uint16_t crc = iso13239_crc_calculate(type, data, data_size);
    bit_buffer_append_bytes(buf, (const uint8_t*)&crc, ISO13239_CRC_SIZE);
}

bool iso13239_crc_check(Iso13239CrcType type, const BitBuffer* buf) {
    furi_check(buf);

    const size_t data_size = bit_buffer_get_size_bytes(buf);
    if(data_size <= ISO13239_CRC_SIZE) return false;

    uint16_t crc_received;
    bit_buffer_write_bytes_mid(
        buf, &crc_received, data_size - ISO13239_CRC_SIZE, ISO13239_CRC_SIZE);

    const uint8_t* data = bit_buffer_get_data(buf);
    const uint16_t crc_calc = iso13239_crc_calculate(type, data, data_size - ISO13239_CRC_SIZE);

    return crc_calc == crc_received;
}

void iso13239_crc_trim(BitBuffer* buf) {
    furi_check(buf);

    const size_t data_size = bit_buffer_get_size_bytes(buf);
    furi_assert(data_size > ISO13239_CRC_SIZE);

    bit_buffer_set_size_bytes(buf, data_size - ISO13239_CRC_SIZE);
}
