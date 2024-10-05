#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t nfc_util_even_parity8(uint8_t data);

uint8_t nfc_util_even_parity32(uint32_t data);

uint8_t nfc_util_odd_parity8(uint8_t data);

void nfc_util_odd_parity(const uint8_t* src, uint8_t* dst, uint8_t len);

#ifdef __cplusplus
}
#endif
