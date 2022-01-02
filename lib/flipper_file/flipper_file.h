/**
 * @file flipper-file.h
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
 * FlipperFile file = flipper_file_alloc(storage);
 * 
 * do {
 *     const uint32_t version = 1;
 *     const char* string_value = "String value";
 *     const uint32_t uint32_value = 1234;
 *     const uint16_t array_size = 4;
 *     const uint8_t* array[array_size] = {0x00, 0x01, 0xFF, 0xA3};
 *     
 *     if(!flipper_file_open_new(file, "/ext/flipper_file_test")) break;
 *     if(!flipper_file_write_header_cstr(file, "Flipper Test File", version)) break;
 *     if(!flipper_file_write_comment_cstr(file, "Just test file")) break;
 *     if(!flipper_file_write_string_cstr(file, "String", string_value)) break;
 *     if(!flipper_file_flipper_file_write_uint32(file, "UINT", &uint32_value, 1)) break;
 *     if(!flipper_file_write_hex(file, "Hex Array", array, array_size)) break;
 *     
 *     // signal that the file was written successfully
 * } while(0);
 * 
 * flipper_file_close(file);
 * flipper_file_free(file);
 * ~~~~~~~~~~~~~~~~~~~~~
 * 
 * Reading:
 * 
 * ~~~~~~~~~~~~~~~~~~~~~
 * FlipperFile file = flipper_file_alloc(storage);
 * 
 * do {
 *     uint32_t version = 1;
 *     string_t file_type;
 *     string_t string_value;
 *     uint32_t uint32_value = 1;
 *     uint16_t array_size = 4;
 *     uint8_t* array[array_size] = {0};
 *     string_init(file_type);
 *     string_init(string_value);
 *     
 *     if(!flipper_file_open_existing(file, "/ext/flipper_file_test")) break;
 *     if(!flipper_file_read_header(file, file_type, &version)) break;
 *     if(!flipper_file_read_string(file, "String", string_value)) break;
 *     if(!flipper_file_read_uint32(file, "UINT", &uint32_value, 1)) break;
 *     if(!flipper_file_read_hex(file, "Hex Array", array, array_size)) break;
 *     
 *     // signal that the file was read successfully
 * } while(0);
 * 
 * flipper_file_close(file);
 * flipper_file_free(file);
 * ~~~~~~~~~~~~~~~~~~~~~
 * 
 */

#pragma once
#include <stdint.h>
#include <mlib/m-string.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

/** FlipperFile type anonymous structure. */
typedef struct FlipperFile FlipperFile;

/**
 * Allocate FlipperFile.
 * @param storage storage api
 * @return FlipperFile* Pointer to a FlipperFile instance
 */
FlipperFile* flipper_file_alloc(Storage* storage);

/**
 * Free FlipperFile.
 * @param flipper_file Pointer to a FlipperFile instance
 */
void flipper_file_free(FlipperFile* flipper_file);

/**
 * Free FlipperFile.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param strict_mode True obligates not to skip valid fields. False by default.
 */
void flipper_file_set_strict_mode(FlipperFile* flipper_file, bool strict_mode);

/**
 * Open existing file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param filename File name and path
 * @return True on success
 */
bool flipper_file_open_existing(FlipperFile* flipper_file, const char* filename);

/**
 * Open existing file for writing and add values to the end of file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param filename File name and path
 * @return True on success
 */
bool flipper_file_open_append(FlipperFile* flipper_file, const char* filename);

/**
 * Open file. Creates a new file, or deletes the contents of the file if it already exists.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param filename File name and path
 * @return True on success
 */
bool flipper_file_open_always(FlipperFile* flipper_file, const char* filename);

/**
 * Open file. Creates a new file, fails if file already exists.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param filename File name and path
 * @return True on success
 */
bool flipper_file_open_new(FlipperFile* flipper_file, const char* filename);

/**
 * Close the file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @return True on success
 */
bool flipper_file_close(FlipperFile* flipper_file);

/**
 * Rewind the file RW pointer.
 * @param flipper_file Pointer to a FlipperFile instance
 * @return True on success
 */
bool flipper_file_rewind(FlipperFile* flipper_file);

/**
 * Read the header (file type and version) from the file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param filetype File type string
 * @param version Version Value
 * @return True on success
 */
bool flipper_file_read_header(FlipperFile* flipper_file, string_t filetype, uint32_t* version);

/**
 * Write the header (file type and version) to the file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param filetype File type string
 * @param version Version Value
 * @return True on success
 */
bool flipper_file_write_header(
    FlipperFile* flipper_file,
    string_t filetype,
    const uint32_t version);

/**
 * Write the header (file type and version) to the file. Plain C string version.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param filetype File type string
 * @param version Version Value
 * @return True on success
 */
bool flipper_file_write_header_cstr(
    FlipperFile* flipper_file,
    const char* filetype,
    const uint32_t version);

/**
 * Get the count of values by key
 * @param flipper_file 
 * @param key 
 * @param count 
 * @return bool 
 */
bool flipper_file_get_value_count(FlipperFile* flipper_file, const char* key, uint32_t* count);

/**
 * Read a string from a file by Key
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_file_read_string(FlipperFile* flipper_file, const char* key, string_t data);

/**
 * Write key and string to file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_file_write_string(FlipperFile* flipper_file, const char* key, string_t data);

/**
 * Write key and string to file. Plain C string version.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_file_write_string_cstr(FlipperFile* flipper_file, const char* key, const char* data);

/**
 * Read array of uint32 from a file by Key
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_read_uint32(
    FlipperFile* flipper_file,
    const char* key,
    uint32_t* data,
    const uint16_t data_size);

/**
 * Write key and array of uint32 to file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_write_uint32(
    FlipperFile* flipper_file,
    const char* key,
    const uint32_t* data,
    const uint16_t data_size);

/**
 * Read array of int32 from a file by Key
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_read_int32(
    FlipperFile* flipper_file,
    const char* key,
    int32_t* data,
    const uint16_t data_size);

/**
 * Write key and array of int32 to file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_write_int32(
    FlipperFile* flipper_file,
    const char* key,
    const int32_t* data,
    const uint16_t data_size);

/**
 * Read array of float from a file by Key
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_read_float(
    FlipperFile* flipper_file,
    const char* key,
    float* data,
    const uint16_t data_size);

/**
 * Write key and array of float to file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_write_float(
    FlipperFile* flipper_file,
    const char* key,
    const float* data,
    const uint16_t data_size);

/**
 * Read hex array from a file by Key
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @param data_size Value size
 * @return True on success
 */
bool flipper_file_read_hex(
    FlipperFile* flipper_file,
    const char* key,
    uint8_t* data,
    const uint16_t data_size);

/**
 * Write key and hex array to file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_write_hex(
    FlipperFile* flipper_file,
    const char* key,
    const uint8_t* data,
    const uint16_t data_size);

/**
 * Write comment to file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param data Comment text
 * @return True on success
 */
bool flipper_file_write_comment(FlipperFile* flipper_file, string_t data);

/**
 * Write comment to file. Plain C string version.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param data Comment text
 * @return True on success
 */
bool flipper_file_write_comment_cstr(FlipperFile* flipper_file, const char* data);

/**
 * Removes the first matching key and its value from the file. Changes the RW pointer to an undefined position.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @return True on success
 */
bool flipper_file_delete_key(FlipperFile* flipper_file, const char* key);

/**
 * Updates the value of the first matching key to a string value. Changes the RW pointer to an undefined position.
 * @param flipper_file Pointer to a FlipperFile instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_file_update_string(FlipperFile* flipper_file, const char* key, string_t data);

/**
 * Updates the value of the first matching key to a string value. Plain C version. Changes the RW pointer to an undefined position.
 * @param flipper_file Pointer to a FlipperFile instance 
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_file_update_string_cstr(FlipperFile* flipper_file, const char* key, const char* data);

/**
 * Updates the value of the first matching key to a uint32 array value. Changes the RW pointer to an undefined position.
 * @param flipper_file Pointer to a FlipperFile instance 
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_update_uint32(
    FlipperFile* flipper_file,
    const char* key,
    const uint32_t* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to a int32 array value. Changes the RW pointer to an undefined position.
 * @param flipper_file Pointer to a FlipperFile instance 
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_update_int32(
    FlipperFile* flipper_file,
    const char* key,
    const int32_t* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to a float array value. Changes the RW pointer to an undefined position.
 * @param flipper_file Pointer to a FlipperFile instance 
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_update_float(
    FlipperFile* flipper_file,
    const char* key,
    const float* data,
    const uint16_t data_size);

/**
 * Updates the value of the first matching key to a hex array value. Changes the RW pointer to an undefined position.
 * @param flipper_file Pointer to a FlipperFile instance 
 * @param key Key
 * @param data Value
 * @param data_size Values count
 * @return True on success
 */
bool flipper_file_update_hex(
    FlipperFile* flipper_file,
    const char* key,
    const uint8_t* data,
    const uint16_t data_size);

/** Get file descriptor.
 * 
 * We higly don't recommend to use it.
 * This instance is owned by FlipperFile.
 * @param flipper_file 
 * @return File* 
 */
File* flipper_file_get_file(FlipperFile* flipper_file);

#ifdef __cplusplus
}
#endif
