#pragma once
#include <stdint.h>
#include <mlib/m-string.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const char flipper_file_eoln;
extern const char flipper_file_eolr;

/**
 * Negative seek helper
 * @param file 
 * @param offset 
 * @return bool 
 */
bool file_helper_seek(File* file, int32_t offset);

/**
 * Writes data to a file as a hexadecimal array.
 * @param file 
 * @param data 
 * @param data_size 
 * @return true on success write 
 */
bool file_helper_write_hex(File* file, const uint8_t* data, const uint16_t data_size);

/**
 * Reads data as a string from the stored rw pointer to the \\n symbol position. Ignores \r.
 * @param file 
 * @param str_result 
 * @return true on success read
 */
bool file_helper_read_line(File* file, string_t str_result);

/**
 * Moves the RW pointer to the beginning of the next line
 * @param file 
 * @return bool 
 */
bool file_helper_seek_to_next_line(File* file);

/**
 * Read one value from array-like string (separated by ' ')
 * @param file 
 * @param value 
 * @return bool 
 */
bool file_helper_read_value(File* file, string_t value, bool* last);

/**
 * Write helper
 * @param file 
 * @param data 
 * @param data_size 
 * @return bool 
 */
bool file_helper_write(File* file, const void* data, uint16_t data_size);

/**
 * Write EOL
 * @param file 
 * @return bool 
 */
bool file_helper_write_eol(File* file);

/**
 * Appends part of one file to the end of another file
 * @param file_from 
 * @param file_to 
 * @param start_offset 
 * @param stop_offset 
 * @return bool 
 */
bool file_helper_copy(File* file_from, File* file_to, uint64_t start_offset, uint64_t stop_offset);

#ifdef __cplusplus
}
#endif