#pragma once
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Convert ASCII hex value to nibble
 * @param c         ASCII character
 * @param nibble    nibble pointer, output
 *
 * @return          bool conversion status
 */
bool hex_char_to_hex_nibble(char c, uint8_t* nibble);

/** Convert ASCII hex value to byte
 * @param hi        hi nibble text
 * @param low       low nibble text
 * @param value     output value
 *
 * @return          bool conversion status
 */
bool hex_char_to_uint8(char hi, char low, uint8_t* value);

/** Convert ASCII hex values to uint8_t
 * @param value_str ASCII data
 * @param value     output value
 *
 * @return          bool conversion status
 */
bool hex_chars_to_uint8(const char* value_str, uint8_t* value);

/** Convert ASCII hex values to uint64_t
 * @param value_str ASCII 64 bi data
 * @param value     output value
 *
 * @return          bool conversion status
 */
bool hex_chars_to_uint64(const char* value_str, uint64_t* value);

/** Convert uint8_t to ASCII hex values
 * @param src       source data
 * @param target    output value
 * @param length    data length
 * 
 */
void uint8_to_hex_chars(const uint8_t* src, uint8_t* target, int length);

#ifdef __cplusplus
}
#endif
