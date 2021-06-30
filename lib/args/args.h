#pragma once
#include "m-string.h"
#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extract first word from arguments string and trim arguments string
 * 
 * @param args arguments string
 * @param word first word, output
 * @return true - success
 * @return false - arguments string does not contain anything
 */
bool args_read_string_and_trim(string_t args, string_t word);

/**
 * @brief Convert hex ASCII values to byte array
 * 
 * @param args arguments string
 * @param bytes byte array pointer, output
 * @param bytes_count needed bytes count
 * @return true - success
 * @return false - arguments string does not contain enough values, or contain non-hex ASCII values
 */
bool args_read_hex_bytes(string_t args, uint8_t* bytes, uint8_t bytes_count);

/************************************ HELPERS ***************************************/

/**
 * @brief Get length of first word from arguments string
 * 
 * @param args arguments string
 * @return size_t length of first word
 */
size_t args_get_first_word_length(string_t args);

/**
 * @brief Get length of arguments string
 * 
 * @param args arguments string
 * @return size_t length of arguments string
 */
size_t args_length(string_t args);

/**
 * @brief Convert ASCII hex values to byte
 * 
 * @param hi_nibble ASCII hi nibble character
 * @param low_nibble ASCII low nibble character
 * @param byte byte pointer, output
 * @return bool conversion status
 */
bool args_char_to_hex(char hi_nibble, char low_nibble, uint8_t* byte);

#ifdef __cplusplus
}
#endif