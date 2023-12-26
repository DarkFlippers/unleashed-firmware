#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    KeysDictModeOpenExisting,
    KeysDictModeOpenAlways,
} KeysDictMode;

typedef struct KeysDict KeysDict;

/** Check if the file list exists
 *
 * @param path      - list path
 *
 * @return true if list exists, false otherwise
*/
bool keys_dict_check_presence(const char* path);

/** Open or create list
 * Depending on mode, list will be opened or created.
 *
 * @param path      - Path of the file that contain the list
 * @param mode      - ListKeysMode value
 * @param key_size  - Size of each key in bytes
 *
 * @return Returns KeysDict list instance
*/
KeysDict* keys_dict_alloc(const char* path, KeysDictMode mode, size_t key_size);

/** Close list
 *
 * @param instance  - KeysDict list instance
*/
void keys_dict_free(KeysDict* instance);

/** Get total number of keys in list
 *
 * @param instance  - KeysDict list instance
 *
 * @return Returns total number of keys in list
*/
size_t keys_dict_get_total_keys(KeysDict* instance);

/** Rewind list
 *
 * @param instance  - KeysDict list instance
 *
 * @return Returns true if rewind was successful, false otherwise
*/
bool keys_dict_rewind(KeysDict* instance);

/** Check if key is present in list
 *
 * @param instance  - KeysDict list instance
 * @param key       - key to check
 * @param key_size  - Size of the key in bytes
 *
 * @return Returns true if key is present, false otherwise
*/
bool keys_dict_is_key_present(KeysDict* instance, const uint8_t* key, size_t key_size);

/** Get next key from the list
 * This function will return next key from list. If there are no more
 * keys, it will return false, and keys_dict_rewind() should be called.
 *
 * @param instance  - KeysDict list instance
 * @param key       - Array where to store key
 * @param key_size  - Size of key in bytes
 *
 * @return Returns true if key was successfully retrieved, false otherwise
*/
bool keys_dict_get_next_key(KeysDict* instance, uint8_t* key, size_t key_size);

/** Add key to list
 *
 * @param instance  - KeysDict list instance
 * @param key       - Key to add
 * @param key_size  - Size of the key in bytes
 *
 * @return Returns true if key was successfully added, false otherwise
*/
bool keys_dict_add_key(KeysDict* instance, const uint8_t* key, size_t key_size);

/** Delete key from list
 *
 * @param instance  - KeysDict list instance
 * @param key       - Key to delete
 * @param key_size  - Size of the key in bytes
 *
 * @return Returns true if key was successfully deleted, false otherwise
*/
bool keys_dict_delete_key(KeysDict* instance, const uint8_t* key, size_t key_size);

#ifdef __cplusplus
}
#endif
