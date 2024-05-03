#include "pretty_format.h"

#include <core/check.h>
#include <core/core_defines.h>

#define PRETTY_FORMAT_MAX_CANONICAL_DATA_SIZE 256U

void pretty_format_bytes_hex_canonical(
    FuriString* result,
    size_t num_places,
    const char* line_prefix,
    const uint8_t* data,
    size_t data_size) {
    furi_check(data);

    bool is_truncated = false;

    if(data_size > PRETTY_FORMAT_MAX_CANONICAL_DATA_SIZE) {
        data_size = PRETTY_FORMAT_MAX_CANONICAL_DATA_SIZE;
        is_truncated = true;
    }

    /* Only num_places byte(s) can be on a single line, therefore: */
    const size_t line_count =
        data_size / num_places + (data_size % num_places != 0 ? 1 : 0) + (is_truncated ? 2 : 0);
    /* Line length = Prefix length + 3 * num_places (2 hex digits + space) + 1 * num_places +
       + 1 pipe character + 1 newline character */
    const size_t line_length = (line_prefix ? strlen(line_prefix) : 0) + 4 * num_places + 2;

    /* Reserve memory in adance in order to avoid unnecessary reallocs */
    furi_string_reserve(result, furi_string_size(result) + line_count * line_length);

    for(size_t i = 0; i < data_size; i += num_places) {
        if(line_prefix) {
            furi_string_cat(result, line_prefix);
        }

        const size_t begin_idx = i;
        const size_t end_idx = MIN(i + num_places, data_size);

        for(size_t j = begin_idx; j < end_idx; j++) {
            furi_string_cat_printf(result, "%02X ", data[j]);
        }

        furi_string_push_back(result, '|');

        for(size_t j = begin_idx; j < end_idx; j++) {
            const char c = data[j];
            const char sep = ((j < end_idx - 1) ? ' ' : '\n');
            const char* fmt = ((j < data_size - 1) ? "%c%c" : "%c");
            furi_string_cat_printf(result, fmt, (c > 0x1f && c < 0x7f) ? c : '.', sep);
        }
    }

    if(is_truncated) {
        furi_string_cat_printf(
            result, "\n(Data is too big. Showing only the first %zu bytes.)", data_size);
    }
}
