#pragma once

#include <stdint.h>
#include <stddef.h>

#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISO14443_CRC_SIZE sizeof(uint16_t)

typedef enum {
    Iso14443CrcTypeA,
    Iso14443CrcTypeB,
} Iso14443CrcType;

void iso14443_crc_append(Iso14443CrcType type, BitBuffer* buf);

bool iso14443_crc_check(Iso14443CrcType type, const BitBuffer* buf);

void iso14443_crc_trim(BitBuffer* buf);

#ifdef __cplusplus
}
#endif
