#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Extract int value and trim arguments string
 *
 * @param args - arguments string
 * @param value first argument, output
 * @return true - success
 * @return false - arguments string does not contain int
 */
bool args_read_int_and_trim(FuriString* args, int* value);

/** Extract float value and trim arguments string
 *
 * @param [in, out] args arguments string
 * @param [out] value first argument
 * @return true - success
 * @return false - arguments string does not contain float
 */
bool args_read_float_and_trim(FuriString* args, float* value);

/**
 * @brief Extract first argument from arguments string and trim arguments string
 *
 * @param args arguments string
 * @param word first argument, output
 * @return true - success
 * @return false - arguments string does not contain anything
 */
bool args_read_string_and_trim(FuriString* args, FuriString* word);

/**
 * @brief Extract the first quoted argument from the argument string and trim the argument string. If the argument is not quoted, calls args_read_string_and_trim.
 *
 * @param args arguments string
 * @param word first argument, output, without quotes
 * @return true - success
 * @return false - arguments string does not contain anything
 */
bool args_read_probably_quoted_string_and_trim(FuriString* args, FuriString* word);

/**
 * @brief Convert hex ASCII values to byte array
 *
 * @param args arguments string
 * @param bytes byte array pointer, output
 * @param bytes_count needed bytes count
 * @return true - success
 * @return false - arguments string does not contain enough values, or contain non-hex ASCII values
 */
bool args_read_hex_bytes(FuriString* args, uint8_t* bytes, size_t bytes_count);

/**
 * @brief Parses a duration value from a given string and converts it to milliseconds
 *
 * @param [in] args the input string containing the duration value. The string may include units (e.g., "10s", "0.5m").
 * @param [out] value pointer to store the parsed value in milliseconds
 * @param [in] default_unit  A default unit to be used if the input string does not contain a valid suffix.
 *                          Supported units: `"ms"`, `"s"`, `"m"`, `"h"`
 *                          If NULL, the function will assume milliseconds by default.
 * @return `true` if the parsing and conversion succeeded, `false` otherwise.
 */
bool args_read_duration(FuriString* args, uint32_t* value, const char* default_unit);

/************************************ HELPERS ***************************************/

/**
 * @brief Get length of first word from arguments string
 *
 * @param args arguments string
 * @return size_t length of first word
 */
size_t args_get_first_word_length(FuriString* args);

/**
 * @brief Get length of arguments string
 *
 * @param args arguments string
 * @return size_t length of arguments string
 */
size_t args_length(FuriString* args);

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
