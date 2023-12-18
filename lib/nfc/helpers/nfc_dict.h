#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NfcDictModeOpenExisting,
    NfcDictModeOpenAlways,
} NfcDictMode;

typedef struct NfcDict NfcDict;

/** Check dictionary presence
 *
 * @param path      - dictionary path
 *
 * @return true if dictionary exists, false otherwise
*/
bool nfc_dict_check_presence(const char* path);

/** Open or create dictionary
 * Depending on mode, dictionary will be opened or created.
 *
 * @param path      - dictionary path
 * @param mode      - NfcDictMode value
 * @param key_size  - size of dictionary keys in bytes
 *
 * @return NfcDict dictionary instance
*/
NfcDict* nfc_dict_alloc(const char* path, NfcDictMode mode, size_t key_size);

/** Close dictionary
 *
 * @param instance  - NfcDict dictionary instance
*/
void nfc_dict_free(NfcDict* instance);

/** Get total number of keys in dictionary
 *
 * @param instance  - NfcDict dictionary instance
 *
 * @return total number of keys in dictionary
*/
uint32_t nfc_dict_get_total_keys(NfcDict* instance);

/** Rewind dictionary
 *
 * @param instance  - NfcDict dictionary instance
 *
 * @return true if rewind was successful, false otherwise
*/
bool nfc_dict_rewind(NfcDict* instance);

/** Check if key is present in dictionary
 *
 * @param instance  - NfcDict dictionary instance
 * @param key       - key to check
 * @param key_size  - size of key in bytes
 *
 * @return true if key is present, false otherwise
*/
bool nfc_dict_is_key_present(NfcDict* instance, const uint8_t* key, size_t key_size);

/** Get next key from dictionary
 * This function will return next key from dictionary. If there are no more
 * keys, it will return false, and nfc_dict_rewind() should be called.
 *
 * @param instance  - NfcDict dictionary instance
 * @param key       - buffer to store key
 * @param key_size  - size of key in bytes
 *
 * @return true if key was successfully retrieved, false otherwise
*/
bool nfc_dict_get_next_key(NfcDict* instance, uint8_t* key, size_t key_size);

/** Add key to dictionary
 *
 * @param instance  - NfcDict dictionary instance
 * @param key       - key to add
 * @param key_size  - size of key in bytes
 *
 * @return true if key was successfully added, false otherwise
*/
bool nfc_dict_add_key(NfcDict* instance, const uint8_t* key, size_t key_size);

/** Delete key from dictionary
 *
 * @param instance  - NfcDict dictionary instance
 * @param key       - key to delete
 * @param key_size  - size of key in bytes
 *
 * @return true if key was successfully deleted, false otherwise
*/
bool nfc_dict_delete_key(NfcDict* instance, const uint8_t* key, size_t key_size);

#ifdef __cplusplus
}
#endif
