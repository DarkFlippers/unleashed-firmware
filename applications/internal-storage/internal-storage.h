#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Internal storage state */
typedef enum {
    InternalStorageStateInitializing,
    InternalStorageStateReady,
    InternalStorageStateBroken,
} InternalStorageState;

typedef struct InternalStorage InternalStorage;

/** Read key, blocking api
 * @param internal_storage - InternalStorage instance
 * @param key - file name to read data from
 * @param buffer - pointer to data buffer
 * @param size - buffer size
 * @return negative on error, otherwise data read
 */
int internal_storage_read_key(
    InternalStorage* internal_storage,
    const char* key,
    uint8_t* buffer,
    size_t size);

/** Write key, blocking api
 * @param internal_storage - InternalStorage instance
 * @param key - file name to store data to
 * @param buffer - pointer to data buffer
 * @param size - buffer size
 * @return negative on error, otherwise data written
 */
int internal_storage_write_key(
    InternalStorage* internal_storage,
    const char* key,
    uint8_t* buffer,
    size_t size);
