#pragma once

#include <stdint.h>

void nfc_util_num2bytes(uint64_t src, uint8_t len, uint8_t* dest);

uint64_t nfc_util_bytes2num(uint8_t* src, uint8_t len);

uint8_t nfc_util_even_parity32(uint32_t data);

uint8_t nfc_util_odd_parity8(uint8_t data);
