#pragma once

#include <m-string.h>
#include <m-array.h>
#include <stdint.h>

typedef struct {
    string_t name;
    uint64_t key;
    uint16_t type;
} SubGhzKey;

ARRAY_DEF(SubGhzKeyArray, SubGhzKey, M_POD_OPLIST)

#define M_OPL_SubGhzKeyArray_t() ARRAY_OPLIST(SubGhzKeyArray, M_POD_OPLIST)

typedef struct SubGhzKeystore SubGhzKeystore;

/** Allocate SubGhzKeystore
 * 
 * @return SubGhzKeystore* 
 */
SubGhzKeystore* subghz_keystore_alloc();

/** Free SubGhzKeystore
 * 
 * @param instance 
 */
void subghz_keystore_free(SubGhzKeystore* instance);

/** Loading manufacture key from file
 * 
 * @param instance - SubGhzKeystore instance
 * @param filename - const char* full path to the file
 */
bool subghz_keystore_load(SubGhzKeystore* instance, const char* filename);

/** Save manufacture key to file
 * 
 * @param instance - SubGhzKeystore instance
 * @param filename - const char* full path to the file
 */
bool subghz_keystore_save(SubGhzKeystore* instance, const char* filename, uint8_t* iv);

/** Get array of keys and names manufacture
 * 
 * @param instance - SubGhzKeystore instance
 * @return SubGhzKeyArray_t*
 */
SubGhzKeyArray_t* subghz_keystore_get_data(SubGhzKeystore* instance);

/** Save RAW encrypted to file
 * 
 * @param input_file_name - const char* full path to the input file
 * @param output_file_name - const char* full path to the output file
 */
bool subghz_keystore_raw_encrypted_save(
    const char* input_file_name,
    const char* output_file_name,
    uint8_t* iv);

/** Get decrypt RAW data to file
 * 
 * @param file_name - const char* full path to the input file
 * @param offset - offset from the start of the RAW data
 * @param data - returned array
 * @param len - required data length
 */
bool subghz_keystore_raw_get_data(const char* file_name, size_t offset, uint8_t* data, size_t len);
