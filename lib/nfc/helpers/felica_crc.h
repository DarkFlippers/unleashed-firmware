#pragma once

#include <stdint.h>
#include <stddef.h>

#include "bit_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FELICA_CRC_SIZE sizeof(uint16_t)

void felica_crc_append(BitBuffer* buf);

bool felica_crc_check(const BitBuffer* buf);

void felica_crc_trim(BitBuffer* buf);

#ifdef __cplusplus
}
#endif
