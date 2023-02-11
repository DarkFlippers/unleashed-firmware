#pragma once

#include <furi.h>

#include "subghz_keystore.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SubGhzEnvironment SubGhzEnvironment;

/**
 * Allocate SubGhzEnvironment.
 * @return SubGhzEnvironment* pointer to a SubGhzEnvironment instance
 */
SubGhzEnvironment* subghz_environment_alloc();

/**
 * Free SubGhzEnvironment.
 * @param instance Pointer to a SubGhzEnvironment instance
 */
void subghz_environment_free(SubGhzEnvironment* instance);

/**
 * Downloading the manufacture key file.
 * @param instance Pointer to a SubGhzEnvironment instance
 * @param filename Full path to the file
 * @return true On success
 */
bool subghz_environment_load_keystore(SubGhzEnvironment* instance, const char* filename);

/**
 * Get pointer to a SubGhzKeystore* instance.
 * @return SubGhzEnvironment* pointer to a SubGhzEnvironment instance
 */
SubGhzKeystore* subghz_environment_get_keystore(SubGhzEnvironment* instance);

/**
 * Set filename to work with Came Atomo.
 * @param instance Pointer to a SubGhzEnvironment instance
 * @param filename Full path to the file
 */
void subghz_environment_set_came_atomo_rainbow_table_file_name(
    SubGhzEnvironment* instance,
    const char* filename);

/**
 * Get filename to work with Came Atomo.
 * @param instance Pointer to a SubGhzEnvironment instance
 * @return Full path to the file
 */
const char* subghz_environment_get_came_atomo_rainbow_table_file_name(SubGhzEnvironment* instance);

/**
 * Set filename to work with Alutech at-4n.
 * @param instance Pointer to a SubGhzEnvironment instance
 * @param filename Full path to the file
 */
void subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
    SubGhzEnvironment* instance,
    const char* filename);

/**
 * Get filename to work with Alutech at-4n.
 * @param instance Pointer to a SubGhzEnvironment instance
 * @return Full path to the file
 */
const char*
    subghz_environment_get_alutech_at_4n_rainbow_table_file_name(SubGhzEnvironment* instance);

/**
 * Set filename to work with Nice Flor-S.
 * @param instance Pointer to a SubGhzEnvironment instance
 * @param filename Full path to the file
 */
void subghz_environment_set_nice_flor_s_rainbow_table_file_name(
    SubGhzEnvironment* instance,
    const char* filename);

/**
 * Get filename to work with Nice Flor-S.
 * @param instance Pointer to a SubGhzEnvironment instance
 * @return Full path to the file
 */
const char*
    subghz_environment_get_nice_flor_s_rainbow_table_file_name(SubGhzEnvironment* instance);

/**
 * Set list of protocols to work.
 * @param instance Pointer to a SubGhzEnvironment instance
 * @param protocol_registry_items Pointer to a SubGhzProtocolRegistry
 */
void subghz_environment_set_protocol_registry(
    SubGhzEnvironment* instance,
    void* protocol_registry_items);

/**
 * Get list of protocols to work.
 * @param instance Pointer to a SubGhzEnvironment instance
 * @return Pointer to a SubGhzProtocolRegistry
 */
void* subghz_environment_get_protocol_registry(SubGhzEnvironment* instance);

/**
 * Get list of protocols names.
 * @param instance Pointer to a SubGhzEnvironment instance
 * @param idx index protocols
 * @return Pointer to a SubGhzProtocolRegistry
 */
const char* subghz_environment_get_protocol_name_registry(SubGhzEnvironment* instance, size_t idx);

#ifdef __cplusplus
}
#endif
