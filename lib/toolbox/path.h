#pragma once

#include <m-string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Extract filename without extension from path.
 * 
 * @param path path string
 * @param filename output filename string. Must be initialized before.
 */
void path_extract_filename_no_ext(const char* path, string_t filename);

#ifdef __cplusplus
}
#endif