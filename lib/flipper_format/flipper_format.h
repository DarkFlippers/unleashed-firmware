/**
 * @file flipper_format.h
 * Flipper File Format helper library.
 * 
 * Flipper File Format is a fairly simple format for storing data in a file.
 * 
 * Flipper file structure:
 * 
 * ~~~~~~~~~~~~~~~~~~~~~
 * # Commentary
 * Field name: field value
 * ~~~~~~~~~~~~~~~~~~~~~
 * 
 * Lines starting with the # character are ignored (considered as comments). The separator between the name of the value and the value itself is the string ": ".
 *
 * Currently supported types:
 * 
 * ~~~~~~~~~~~~~~~~~~~~~
 * String: text
 * Int32: 1 2 -3 4
 * Uint32: 1 2 3 4
 * Float: 1.0 1234.654
 * Hex: A4 B3 C2 D1 12 FF
 * ~~~~~~~~~~~~~~~~~~~~~
 * 
 * End of line is LF when writing, but CR is supported when reading.
 * 
 * The library is designed in such a way that comments and field values are completely ignored when searching for keys, that is, they do not consume memory.
 * 
 * File example: 
 * 
 * ~~~~~~~~~~~~~~~~~~~~~
 * Filetype: Flipper Test File
 * Version: 1
 * # Just test file
 * String: String value
 * UINT: 1234
 * Hex: 00 01 FF A3
 * ~~~~~~~~~~~~~~~~~~~~~
 * 
 * Writing:
 * 
 * ~~~~~~~~~~~~~~~~~~~~~
 * FlipperFormat* format = flipper_format_file_alloc(storage);
 * 
 * do {
 *     const uint32_t version = 1;
 *     const char* string_value = "String value";
 *     const uint32_t uint32_value = 1234;
 *     const uint16_t array_size = 4;
 *     const uint8_t* array[array_size] = {0x00, 0x01, 0xFF, 0xA3};
 *     
 *     if(!flipper_format_file_open_new(format, EXT_PATH("flipper_format_test"))) break;
 *     if(!flipper_format_write_header_cstr(format, "Flipper Test File", version)) break;
 *     if(!flipper_format_write_comment_cstr(format, "Just test file")) break;
 *     if(!flipper_format_write_string_cstr(format, "String", string_value)) break;
 *     if(!flipper_format_write_uint32(format, "UINT", &uint32_value, 1)) break;
 *     if(!flipper_format_write_hex(format, "Hex Array", array, array_size)) break;
 *     
 *     // signal that the file was written successfully
 * } while(0);
 * 
 * flipper_format_free(file);
 * ~~~~~~~~~~~~~~~~~~~~~
 * 
 * Reading:
 * 
 * ~~~~~~~~~~~~~~~~~~~~~
 * FlipperFormat* file = flipper_format_file_alloc(storage);
 * 
 * do {
 *     uint32_t version = 1;
 *     FuriString* file_type;
 *     FuriString* string_value;
 *     uint32_t uint32_value = 1;
 *     uint16_t array_size = 4;
 *     uint8_t* array[array_size] = {0};
 *     file_type = furi_string_alloc();
 *     string_value = furi_string_alloc();
 *     
 *     if(!flipper_format_file_open_existing(file, EXT_PATH("flipper_format_test"))) break;
 *     if(!flipper_format_read_header(file, file_type, &version)) break;
 *     if(!flipper_format_read_string(file, "String", string_value)) break;
 *     if(!flipper_format_read_uint32(file, "UINT", &uint32_value, 1)) break;
 *     if(!flipper_format_read_hex(file, "Hex Array", array, array_size)) break;
 *     
 *     // signal that the file was read successfully
 * } while(0);
 * 
 * flipper_format_free(file);
 * ~~~~~~~~~~~~~~~~~~~~~
 * 
 */

#pragma once
#include <stdint.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FlipperFormat FlipperFormat;

/**
 * Allocate FlipperFormat as string.
 * @return FlipperFormat* pointer to a FlipperFormat instance
 */
FlipperFormat* flipper_format_string_alloc();

/**
 * Allocate FlipperFormat as file.
 * @return FlipperFormat* pointer to a FlipperFormat instance
 */
FlipperFormat* flipper_format_file_alloc(Storage* storage);

/**
 * Allocate FlipperFormat as file, buffered mode.
 * @return FlipperFormat* pointer to a FlipperFormat instance
 */
FlipperFormat* flipper_format_buffered_file_alloc(Storage* storage);

/**
 * Open existing file. 
 * Use only if FlipperFormat allocated as a file.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param path File path
 * @return True on success
 */
bool flipper_format_file_open_existing(FlipperFormat* flipper_format, const char* path);

/**
 * Open existing file, buffered mode.
 * Use only if FlipperFormat allocated as a buffered file.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param path File path
 * @return True on success
 */
bool flipper_format_buffered_file_open_existing(FlipperFormat* flipper_format, const char* path);

/**
 * Open existing file for writing and add values to the end of file. 
 * Use only if FlipperFormat allocated as a file.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param path File path
 * @return True on success
 */
bool flipper_format_file_open_append(FlipperFormat* flipper_format, const char* path);

/**
 * Open file. Creates a new file, or deletes the contents of the file if it already exists. 
 * Use only if FlipperFormat allocated as a file.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param path File path
 * @return True on success
 */
bool flipper_format_file_open_always(FlipperFormat* flipper_format, const char* path);

/**
 * Open file. Creates a new file, or deletes the contents of the file if it already exists, buffered mode.
 * Use only if FlipperFormat allocated as a buffered file.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param path File path
 * @return True on success
 */
bool flipper_format_buffered_file_open_always(FlipperFormat* flipper_format, const char* path);

/**
 * Open file. Creates a new file, fails if file already exists.
 * Use only if FlipperFormat allocated as a file.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param path File path
 * @return True on success
 */
bool flipper_format_file_open_new(FlipperFormat* flipper_format, const char* path);

/**
 * Closes the file, use only if FlipperFormat allocated as a file.
 * @param flipper_format 
 * @return true 
 * @return false 
 */
bool flipper_format_file_close(FlipperFormat* flipper_format);

/**
 * Closes the file, use only if FlipperFormat allocated as a buffered file.
 * @param flipper_format
 * @return true
 * @return false
 */
bool flipper_format_buffered_file_close(FlipperFormat* flipper_format);

/**
 * Free FlipperFormat.
 * @param flipper_format Pointer to a FlipperFormat instance
 */
void flipper_format_free(FlipperFormat* flipper_format);

/**
 * Set FlipperFormat mode.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param strict_mode True obligates not to skip valid fields. False by default.
 */
void flipper_format_set_strict_mode(FlipperFormat* flipper_format, bool strict_mode);

/**
 * Rewind the RW pointer.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return True on success
 */
bool flipper_format_rewind(FlipperFormat* flipper_format);

/**
 * Move the RW pointer at the end. Can be useful if you want to add some data after reading.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return True on success
 */
bool flipper_format_seek_to_end(FlipperFormat* flipper_format);

/**
 * Check if the key exists.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @return true key exists
 * @return false key is not exists
 */
bool flipper_format_key_exist(FlipperFormat* flipper_format, const char* key);

/**
 * Read the header (file type and version).
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param filetype File type string
 * @param version Version Value
 * @return True on success
 */
bool flipper_format_read_header(
    FlipperFormat* flipper_format,
    FuriString* filetype,
    uint32_t* version);

/**
 * Write the header (file type and version).
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param filetype File type string
 * @param version Version Value
 * @return True on success
 */
bool flipper_format_write_header(
    FlipperFormat* flipper_format,
    FuriString* filetype,
    const uint32_t version);

/**
 * Write the header (file type and version). Plain C string version.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param filetype File type string
 * @param version Version Value
 * @return True on success
 */
bool flipper_format_write_header_cstr(
    FlipperFormat* flipper_format,
    const char* filetype,
    const uint32_t version);

/**
 * Get the count of values by key
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key 
 * @param count 
 * @return bool 
 */
bool flipper_format_get_value_count(
    FlipperFormat* flipper_format,
    const char* key,
    uint32_t* count);

/**
 * Read a string by key
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_read_string(FlipperFormat* flipper_format, const char* key, FuriString* data);

/**
 * Write key and string
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_write_string(FlipperFormat* flipper_format, const char* key, FuriString* data);

/**
 * Write key and string. Plain C string version.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_write_string_cstr(
    FlipperFormat* flipper_format,
    const char* key,
    const char* data);

/**
 * Read array of uint64 in hex format by key
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_read_hex_uint64(
    FlipperFormat* flipper_format,
    const char* key,
    uint64_t* data,
    const uint16_t data_size);

/**
 * Write key and array of uint64 in hex format
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_write_hex_uint64(
    FlipperFormat* flipper_format,
    const char* key,
    const uint64_t* data,
    const uint16_t data_size);

/**
 * Read array of uint32 by key
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_read_uint32(
    FlipperFormat* flipper_format,
    const char* key,
    uint32_t* data,
    const uint16_t data_size);

/**
 * Write key and array of uint32
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_write_uint32(
    FlipperFormat* flipper_format,
    const char* key,
    const uint32_t* data,
    const uint16_t data_size);

/**
 * Read array of int32 by key
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_read_int32(
    FlipperFormat* flipper_format,
    const char* key,
    int32_t* data,
    const uint16_t data_size);

/**
 * Write key and array of int32
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_write_int32(
    FlipperFormat* flipper_format,
    const char* key,
    const int32_t* data,
    const uint16_t data_size);

/**
 * Read array of bool by key
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_read_bool(
    FlipperFormat* flipper_format,
    const char* key,
    bool* data,
    const uint16_t data_size);

/**
 * Write key and array of bool
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_write_bool(
    FlipperFormat* flipper_format,
    const char* key,
    const bool* data,
    const uint16_t data_size);

/**
 * Read array of float by key
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_read_float(
    FlipperFormat* flipper_format,
    const char* key,
    float* data,
    const uint16_t data_size);

/**
 * Write key and array of float
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_write_float(
    FlipperFormat* flipper_format,
    const char* key,
    const float* data,
    const uint16_t data_size);

/**
 * Read array of hex-formatted bytes by key
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_read_hex(
    FlipperFormat* flipper_format,
    const char* key,
    uint8_t* data,
    const uint16_t data_size);

/**
 * Write key and array of hex-formatted bytes
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_format_write_hex(
    FlipperFormat* flipper_format,
    const char* key,
    const uint8_t* data,
    const uint16_t data_size);

/**
 * Write comment
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param data Comment text
 * @return True on success
 */
bool flipper_format_write_comment(FlipperFormat* flipper_format, FuriString* data);

/**
 * Write comment. Plain C string version.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param data Comment text
 * @return True on success
 */
bool flipper_format_write_comment_cstr(FlipperFormat* flipper_format, const char* data);

/**
 * Removes the first matching key and its value. Sets the RW pointer to a position of deleted data.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param key Key
 * @return True on success
 */
bool flipper_format_delete_key(FlipperFormat* flipper_format, const char* key);

/**
 * Updates the value of the first matching key to a string value. Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_update_string(FlipperFormat* flipper_format, const char* key, FuriString* data);

/**
 * Updates the value of the first matching key to a string value. Plain C version. Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_update_string_cstr(
    FlipperFormat* flipper_format,
    const char* key,
    const char* data);

/**
 * Updates the value of the first matching key to a uint32 array value. Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_update_uint32(
    FlipperFormat* flipper_format,
    const char* key,
    const uint32_t* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to a int32 array value. Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_update_int32(
    FlipperFormat* flipper_format,
    const char* key,
    const int32_t* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to a bool array value. Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_update_bool(
    FlipperFormat* flipper_format,
    const char* key,
    const bool* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to a float array value. Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_update_float(
    FlipperFormat* flipper_format,
    const char* key,
    const float* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to an array of hex-formatted bytes. Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_update_hex(
    FlipperFormat* flipper_format,
    const char* key,
    const uint8_t* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to a string value, or adds the key and value if the key did not exist. 
 * Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_insert_or_update_string(
    FlipperFormat* flipper_format,
    const char* key,
    FuriString* data);

/**
 * Updates the value of the first matching key to a string value, or adds the key and value if the key did not exist.  
 * Plain C version. 
 * Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_insert_or_update_string_cstr(
    FlipperFormat* flipper_format,
    const char* key,
    const char* data);

/**
 * Updates the value of the first matching key to a uint32 array value, or adds the key and value if the key did not exist. 
 *  Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_insert_or_update_uint32(
    FlipperFormat* flipper_format,
    const char* key,
    const uint32_t* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to a int32 array value, or adds the key and value if the key did not exist. 
 * Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_insert_or_update_int32(
    FlipperFormat* flipper_format,
    const char* key,
    const int32_t* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to a bool array value, or adds the key and value if the key did not exist. 
 * Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_insert_or_update_bool(
    FlipperFormat* flipper_format,
    const char* key,
    const bool* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to a float array value, or adds the key and value if the key did not exist. 
 * Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_insert_or_update_float(
    FlipperFormat* flipper_format,
    const char* key,
    const float* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to an array of hex-formatted bytes, or adds the key and value if the key did not exist.  
 *Sets the RW pointer to a position at the end of inserted data.
 * @param flipper_format Pointer to a FlipperFormat instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_format_insert_or_update_hex(
    FlipperFormat* flipper_format,
    const char* key,
    const uint8_t* data,
    const uint16_t data_size);

#ifdef __cplusplus
}
#endif
