/**
 * @file str_buffer.h
 * 
 * Allows you to create an owned clone of however many strings that you need,
 * then free all of them at once. Essentially the simplest possible append-only
 * unindexable array of owned C-style strings.
 */
#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief StrBuffer instance
 * 
 * Place this struct directly wherever you want, it doesn't have to be `alloc`ed
 * and `free`d.
 */
typedef struct {
    char** owned_strings;
    size_t n_owned_strings;
} StrBuffer;

/**
 * @brief Makes a owned duplicate of the provided string
 * 
 * @param[in] buffer StrBuffer instance
 * @param[in] str    Input C-style string
 * 
 * @returns C-style string that contains to be valid event after `str` becomes
 *          invalid
 */
const char* str_buffer_make_owned_clone(StrBuffer* buffer, const char* str);

/**
 * @brief Clears all owned duplicates
 * 
 * @param[in] buffer StrBuffer instance
 */
void str_buffer_clear_all_clones(StrBuffer* buffer);

#ifdef __cplusplus
}
#endif
