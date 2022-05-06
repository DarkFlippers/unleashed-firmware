#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAXIM_CRC8_INIT 0

uint8_t maxim_crc8(const uint8_t* data, const uint8_t data_size, const uint8_t crc_init);

#ifdef __cplusplus
}
#endif
