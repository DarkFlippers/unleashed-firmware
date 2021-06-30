#pragma once
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert ASCII hex value to nibble 
 * 
 * @param c ASCII character
 * @param nibble nibble pointer, output
 * @return bool conversion status
 */
bool hex_char_to_hex_nibble(char c, uint8_t* nibble);

#ifdef __cplusplus
}
#endif