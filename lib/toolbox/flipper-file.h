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
 * Uint32: 1
 * Hex Array: A4 B3 C2 D1 12 FF
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
 * Hex Array: 00 01 FF A3
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
 *     if(!flipper_file_new_write(file, "/ext/flipper_file_test")) break;
 *     if(!flipper_file_write_header_cstr(file, "Flipper Test File", version)) break;
 *     if(!flipper_file_write_comment_cstr(file, "Just test file")) break;
 *     if(!flipper_file_write_string_cstr(file, "String", string_value)) break;
 *     if(!flipper_file_flipper_file_write_uint32(file, "UINT", uint32_value)) break;
 *     if(!flipper_file_write_hex_array(file, "Hex Array", array, array_size)) break;
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
 *     if(!flipper_file_open_read(file, "/ext/flipper_file_test")) break;
 *     if(!flipper_file_read_header(file, file_type, &version)) break;
 *     if(!flipper_file_read_string(file, "String", string_value)) break;
 *     if(!flipper_file_read_uint32(file, "UINT", &uint32_value)) break;
 *     if(!flipper_file_read_hex_array(file, "Hex Array", array, array_size)) break;
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
 * Open file for reading.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param filename File name and path
 * @return True on success
 */
bool flipper_file_open_read(FlipperFile* flipper_file, const char* filename);

/**
 * Open file for writing. Creates a new file, or deletes the contents of the file if it already exists.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param filename File name and path
 * @return True on success
 */
bool flipper_file_new_write(FlipperFile* flipper_file, const char* filename);

/**
 * Close the file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @return True on success
 */
bool flipper_file_close(FlipperFile* flipper_file);

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
 * Read uint32 from a file by Key
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_file_read_uint32(FlipperFile* flipper_file, const char* key, uint32_t* data);

/**
 * Write key and uint32 to file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @return True on success
 */
bool flipper_file_write_uint32(FlipperFile* flipper_file, const char* key, const uint32_t data);

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
 * Read hex array from a file by Key
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @param data_size Value size
 * @return True on success
 */
bool flipper_file_read_hex_array(
    FlipperFile* flipper_file,
    const char* key,
    uint8_t* data,
    const uint16_t data_size);

/**
 * Write key and hex array to file.
 * @param flipper_file Pointer to a FlipperFile instance
 * @param key Key
 * @param data Value
 * @param data_size Value size
 * @return True on success
 */
bool flipper_file_write_hex_array(
    FlipperFile* flipper_file,
    const char* key,
    const uint8_t* data,
    const uint16_t data_size);

#ifdef __cplusplus
}
#endif