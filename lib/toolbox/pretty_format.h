#pragma once

#include <core/string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PRETTY_FORMAT_FONT_BOLD      "\e#"
#define PRETTY_FORMAT_FONT_MONOSPACE "\e*"

/**
 * Format a data buffer as a canonical HEX dump
 * @param [out] result pointer to the output string (must be initialised)
 * @param [in] num_places the number of bytes on one line (both as HEX and ASCII)
 * @param [in] line_prefix if not NULL, prepend this string to each line
 * @param [in] data pointer to the input data buffer
 * @param [in] data_size input data size
 */
void pretty_format_bytes_hex_canonical(
    FuriString* result,
    size_t num_places,
    const char* line_prefix,
    const uint8_t* data,
    size_t data_size);

#ifdef __cplusplus
}
#endif
