#pragma once
#include <stdlib.h>
#include <storage/storage.h>
#include "stream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a file stream with buffered read operations
 * @return Stream*
 */
Stream* buffered_file_stream_alloc(Storage* storage);

/**
 * Opens an existing file or creates a new one.
 * @param stream pointer to file stream object.
 * @param path path to file
 * @param access_mode access mode from FS_AccessMode
 * @param open_mode open mode from FS_OpenMode
 * @return True on success, False on failure. You need to close the file even if the open operation failed.
 */
bool buffered_file_stream_open(
    Stream* stream,
    const char* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode);

/**
 * Closes the file.
 * @param stream pointer to file stream object.
 * @return True on success, False on failure.
 */
bool buffered_file_stream_close(Stream* stream);

/**
 * Forces write from cache to the underlying file.
 * @param stream pointer to file stream object.
 * @return True on success, False on failure.
 */
bool buffered_file_stream_sync(Stream* stream);

/**
 * Retrieves the error id from the file object
 * @param stream pointer to stream object.
 * @return FS_Error error id
 */
FS_Error buffered_file_stream_get_error(Stream* stream);

#ifdef __cplusplus
}
#endif
