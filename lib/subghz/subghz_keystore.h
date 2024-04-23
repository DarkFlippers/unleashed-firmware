#pragma once

#include <furi.h>
#include <m-array.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    FuriString* name;
    uint64_t key;
    uint16_t type;
} SubGhzKey;

ARRAY_DEF(SubGhzKeyArray, SubGhzKey, M_POD_OPLIST)

#define M_OPL_SubGhzKeyArray_t() ARRAY_OPLIST(SubGhzKeyArray, M_POD_OPLIST)

typedef struct SubGhzKeystore SubGhzKeystore;

/**
 * Allocate SubGhzKeystore.
 * @return SubGhzKeystore* pointer to a SubGhzKeystore instance
 */
SubGhzKeystore* subghz_keystore_alloc(void);

/**
 * Free SubGhzKeystore.
 * @param instance Pointer to a SubGhzKeystore instance
 */
void subghz_keystore_free(SubGhzKeystore* instance);

/** 
 * Loading manufacture key from file
 * @param instance Pointer to a SubGhzKeystore instance
 * @param filename Full path to the file
 */
bool subghz_keystore_load(SubGhzKeystore* instance, const char* filename);

/** 
 * Save manufacture key to file
 * @param instance Pointer to a SubGhzKeystore instance
 * @param filename Full path to the file
 * @return true On success
 */
bool subghz_keystore_save(SubGhzKeystore* instance, const char* filename, uint8_t* iv);

/** 
 * Get array of keys and names manufacture
 * @param instance Pointer to a SubGhzKeystore instance
 * @return SubGhzKeyArray_t*
 */
SubGhzKeyArray_t* subghz_keystore_get_data(SubGhzKeystore* instance);

/** 
 * Save RAW encrypted to file
 * @param input_file_name Full path to the input file
 * @param output_file_name Full path to the output file
 * @param iv IV, 16 bytes in hex
 */
bool subghz_keystore_raw_encrypted_save(
    const char* input_file_name,
    const char* output_file_name,
    uint8_t* iv);

/** 
 * Get decrypt RAW data to file
 * @param file_name Full path to the input file
 * @param offset Offset from the start of the RAW data
 * @param data Returned array
 * @param len Required data length
 * @return true On success
 */
bool subghz_keystore_raw_get_data(const char* file_name, size_t offset, uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif
