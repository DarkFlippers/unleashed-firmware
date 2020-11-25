#pragma once
#include <stdint.h>

uint8_t maxim_crc8(const uint8_t* data, const uint8_t data_size, const uint8_t crc_init = 0);
uint16_t maxim_crc16(const uint8_t* address, const uint8_t length, const uint16_t init = 0);
uint16_t maxim_crc16(uint8_t value, uint16_t crc);