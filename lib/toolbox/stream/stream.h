#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Stream Stream;

typedef enum {
    StreamOffsetFromCurrent,
    StreamOffsetFromStart,
    StreamOffsetFromEnd,
} StreamOffset;

typedef enum {
    StreamDirectionForward,
    StreamDirectionBackward,
} StreamDirection;

typedef bool (*StreamWriteCB)(Stream* stream, const void* context);

/**
 * Free Stream
 * @param stream Stream instance
 */
void stream_free(Stream* stream);

/**
 * Clean (empty) Stream
 * @param stream Stream instance
 */
void stream_clean(Stream* stream);

/**
 * Indicates that the RW pointer is at the end of the stream
 * @param stream Stream instance
 * @return true if RW pointer is at the end of the stream
 * @return false if RW pointer is not at the end of the stream
 */
bool stream_eof(Stream* stream);

/**
 * Moves the RW pointer.
 * @param stream Stream instance
 * @param offset how much to move the pointer
 * @param offset_type starting from what
 * @return true 
 * @return false 
 */
bool stream_seek(Stream* stream, int32_t offset, StreamOffset offset_type);

/** Seek to next occurrence of the character
 *
 * @param      stream     Pointer to the stream instance
 * @param[in]  c          The Character
 * @param[in]  direction  The Direction
 *
 * @return     true on success
 */
bool stream_seek_to_char(Stream* stream, char c, StreamDirection direction);

/**
 * Gets the value of the RW pointer
 * @param stream Stream instance
 * @return size_t value of the RW pointer
 */
size_t stream_tell(Stream* stream);

/**
 * Gets the size of the stream
 * @param stream Stream instance
 * @return size_t size of the stream
 */
size_t stream_size(Stream* stream);

/**
 * Write N bytes to the stream
 * @param stream Stream instance
 * @param data data to write
 * @param size size of data to be written
 * @return size_t how many bytes was written
 */
size_t stream_write(Stream* stream, const uint8_t* data, size_t size);

/**
 * Read N bytes from stream
 * @param stream Stream instance
 * @param data data to be read
 * @param count size of data to be read
 * @return size_t how many bytes was read
 */
size_t stream_read(Stream* stream, uint8_t* data, size_t count);

/**
 * Delete N chars from the stream and write data by calling write_callback(context)
 * @param stream Stream instance
 * @param delete_size size of data to be deleted
 * @param write_callback write callback
 * @param context write callback context
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_delete_and_insert(
    Stream* stream,
    size_t delete_size,
    StreamWriteCB write_callback,
    const void* context);

/********************************** Some random helpers starts here **********************************/

/**
 * Read line from a stream (supports LF and CRLF line endings)
 * @param stream 
 * @param str_result 
 * @return true if line length is not zero
 * @return false otherwise
 */
bool stream_read_line(Stream* stream, FuriString* str_result);

/**
 * Moves the RW pointer to the start
 * @param stream Stream instance
 */
bool stream_rewind(Stream* stream);

/**
 * Write char to the stream
 * @param stream Stream instance
 * @param c char value
 * @return size_t how many bytes was written
 */
size_t stream_write_char(Stream* stream, char c);

/**
 * Write string to the stream
 * @param stream Stream instance
 * @param string string value
 * @return size_t how many bytes was written
 */
size_t stream_write_string(Stream* stream, FuriString* string);

/**
 * Write const char* to the stream
 * @param stream Stream instance
 * @param string c-string value
 * @return size_t how many bytes was written
 */
size_t stream_write_cstring(Stream* stream, const char* string);

/**
 * Write formatted string to the stream
 * @param stream Stream instance
 * @param format 
 * @param ... 
 * @return size_t how many bytes was written
 */
size_t stream_write_format(Stream* stream, const char* format, ...)
    _ATTRIBUTE((__format__(__printf__, 2, 3)));

/**
 * Write formatted string to the stream, va_list version
 * @param stream Stream instance
 * @param format 
 * @param args 
 * @return size_t how many bytes was written
 */
size_t stream_write_vaformat(Stream* stream, const char* format, va_list args);

/**
 * Insert N chars to the stream, starting at the current pointer.
 * Data will be inserted, not overwritten, so the stream will be increased in size.
 * @param stream Stream instance
 * @param data data to be inserted
 * @param size size of data to be inserted
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_insert(Stream* stream, const uint8_t* data, size_t size);

/**
 * Insert char to the stream
 * @param stream Stream instance
 * @param c char value
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_insert_char(Stream* stream, char c);

/**
 * Insert string to the stream
 * @param stream Stream instance
 * @param string string value
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_insert_string(Stream* stream, FuriString* string);

/**
 * Insert const char* to the stream
 * @param stream Stream instance
 * @param string c-string value
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_insert_cstring(Stream* stream, const char* string);

/**
 * Insert formatted string to the stream
 * @param stream Stream instance
 * @param format 
 * @param ... 
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_insert_format(Stream* stream, const char* format, ...)
    _ATTRIBUTE((__format__(__printf__, 2, 3)));

/**
 * Insert formatted string to the stream, va_list version
 * @param stream Stream instance
 * @param format 
 * @param args 
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_insert_vaformat(Stream* stream, const char* format, va_list args);

/**
 * Delete N chars from the stream and insert char to the stream
 * @param stream Stream instance
 * @param delete_size size of data to be deleted
 * @param c char value
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_delete_and_insert_char(Stream* stream, size_t delete_size, char c);

/**
 * Delete N chars from the stream and insert string to the stream
 * @param stream Stream instance
 * @param delete_size size of data to be deleted
 * @param string string value
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_delete_and_insert_string(Stream* stream, size_t delete_size, FuriString* string);

/**
 * Delete N chars from the stream and insert const char* to the stream
 * @param stream Stream instance
 * @param delete_size size of data to be deleted
 * @param string c-string value
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_delete_and_insert_cstring(Stream* stream, size_t delete_size, const char* string);

/**
 * Delete N chars from the stream and insert formatted string to the stream
 * @param stream Stream instance
 * @param delete_size size of data to be deleted
 * @param format 
 * @param ... 
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_delete_and_insert_format(Stream* stream, size_t delete_size, const char* format, ...)
    _ATTRIBUTE((__format__(__printf__, 3, 4)));

/**
 * Delete N chars from the stream and insert formatted string to the stream, va_list version
 * @param stream Stream instance
 * @param delete_size size of data to be deleted
 * @param format 
 * @param args 
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_delete_and_insert_vaformat(
    Stream* stream,
    size_t delete_size,
    const char* format,
    va_list args);

/**
 * Remove N chars from the stream, starting at the current pointer.
 * The size may be larger than stream size, the stream will be cleared from current RW pointer to the end.
 * @param stream Stream instance
 * @param size how many chars need to be deleted
 * @return true if the operation was successful
 * @return false on error
 */
bool stream_delete(Stream* stream, size_t size);

/**
 * Copy data from one stream to another. Data will be copied from current RW pointer and to current RW pointer.
 * @param stream_from 
 * @param stream_to 
 * @param size 
 * @return size_t 
 */
size_t stream_copy(Stream* stream_from, Stream* stream_to, size_t size);

/**
 * Copy data from one stream to another. Data will be copied from start of one stream and to start of other stream.
 * @param stream_from 
 * @param stream_to 
 * @return size_t 
 */
size_t stream_copy_full(Stream* stream_from, Stream* stream_to);

/**
 * Splits one stream into two others. The original stream will remain untouched.
 * @param stream 
 * @param stream_left 
 * @param stream_right 
 * @return true 
 * @return false 
 */
bool stream_split(Stream* stream, Stream* stream_left, Stream* stream_right);

/**
 * Loads data to the stream from a file. Data will be loaded to the current RW pointer. RW pointer will be moved to the end of the stream.
 * @param stream Stream instance 
 * @param storage 
 * @param path 
 * @return size_t 
 */
size_t stream_load_from_file(Stream* stream, Storage* storage, const char* path);

/**
 * Writes data from a stream to a file. Data will be saved starting from the current RW pointer. RW pointer will be moved to the end of the stream.
 * @param stream Stream instance 
 * @param storage 
 * @param path 
 * @param mode 
 * @return size_t 
 */
size_t stream_save_to_file(Stream* stream, Storage* storage, const char* path, FS_OpenMode mode);

/**
 * Dump stream inner data (size, RW position, content)
 * @param stream Stream instance 
 */
void stream_dump_data(Stream* stream);

#ifdef __cplusplus
}
#endif
