#pragma once
#include <stdint.h>
#include <mlib/m-string.h>
#include <storage/storage.h>
#include "file_helper.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const char* flipper_file_filetype_key;
extern const char* flipper_file_version_key;
extern const char flipper_file_delimiter;
extern const char flipper_file_comment;

/**
 * Reads a valid key from a file as a string.
 * After reading, the rw pointer will be on the flipper_file_delimiter symbol.
 * Optimized not to read comments and values into RAM.
 * @param file 
 * @param key 
 * @return true on success read 
 */
bool flipper_file_read_valid_key(File* file, string_t key);

/**
 * Sets rw pointer to the data after the key
 * @param file 
 * @param key 
 * @return true if key was found 
 */
bool flipper_file_seek_to_key(File* file, const char* key);

/**
 * Write key and key delimiter
 * @param file 
 * @param key 
 * @return bool 
 */
bool flipper_file_write_key(File* file, const char* key);

/**
 * Get scratchpad name and path
 * @param name 
 * @return bool 
 */
bool flipper_file_get_scratchpad_name(const char** name);

#ifdef __cplusplus
}
#endif