#pragma once

#include <stdint.h>
#include <stddef.h>

#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISO13239_CRC_SIZE sizeof(uint16_t)

typedef enum {
    Iso13239CrcTypeDefault,
    Iso13239CrcTypePicopass,
} Iso13239CrcType;

void iso13239_crc_append(Iso13239CrcType type, BitBuffer* buf);

bool iso13239_crc_check(Iso13239CrcType type, const BitBuffer* buf);

void iso13239_crc_trim(BitBuffer* buf);

#ifdef __cplusplus
}
#endif
