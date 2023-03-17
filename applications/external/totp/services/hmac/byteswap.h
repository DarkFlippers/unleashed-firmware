#pragma once

#include <stdint.h>

/**
 * @brief Swap bytes in 32-bit value
 * @param val value to swap bytes in
 * @return Value with bytes swapped
 */
uint32_t swap_uint32(uint32_t val);

/**
 * @brief Swap bytes in 64-bit value
 * @param val value to swap bytes in
 * @return Value with bytes swapped
 */
uint64_t swap_uint64(uint64_t val);
