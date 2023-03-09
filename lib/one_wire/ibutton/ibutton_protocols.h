/**
 * @file ibutton_protocols.h
 *
 * Common interface for accessing various iButton protocols
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#include "protocols/protocol_common.h"

#include "ibutton_key.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct iButtonProtocols iButtonProtocols;

/**
 * Allocate an iButtonProtocols object
 * @return pointer to an iButtonProtocols object
 */
iButtonProtocols* ibutton_protocols_alloc();

/**
 * Destroy an iButtonProtocols object, free resources
 * @param [in] protocols pointer to an iButtonProtocols object
 */
void ibutton_protocols_free(iButtonProtocols* protocols);

/**
 * Get the total number of available protocols
 */
uint32_t ibutton_protocols_get_protocol_count();

/**
 * Get maximum data size out of all protocols available
 * @param [in] protocols pointer to an iButtonProtocols object
 * @return maximum data size in bytes
 */
size_t ibutton_protocols_get_max_data_size(iButtonProtocols* protocols);

/**
 * Get the protocol id based on its name
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] name pointer to a string containing the name
 * @return protocol id on success on iButtonProtocolIdInvalid on failure
 */
iButtonProtocolId ibutton_protocols_get_id_by_name(iButtonProtocols* protocols, const char* name);

/**
 * Get the manufacturer name based on the protocol id
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] id id of the protocol in question
 * @return pointer to a statically allocated string with manufacturer name
 */
const char* ibutton_protocols_get_manufacturer(iButtonProtocols* protocols, iButtonProtocolId id);

/**
 * Get the protocol name based on the protocol id
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] id id of the protocol in question
 * @return pointer to a statically allocated string with protocol name
 */
const char* ibutton_protocols_get_name(iButtonProtocols* protocols, iButtonProtocolId id);

/**
 * Get protocol features bitmask by protocol id
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] id id of the protocol in question
 */
uint32_t ibutton_protocols_get_features(iButtonProtocols* protocols, iButtonProtocolId id);

/**
 * Read a physical device (a key or an emulator)
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [out] key pointer to the key to read into (must be allocated before)
 * @return true on success, false on failure
 */
bool ibutton_protocols_read(iButtonProtocols* protocols, iButtonKey* key);

/**
 * Write the key to a blank
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] key pointer to the key to be written
 * @return true on success, false on failure
 */
bool ibutton_protocols_write_blank(iButtonProtocols* protocols, iButtonKey* key);

/**
 * Write the key to another one of the same type
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] key pointer to the key to be written
 * @return true on success, false on failure
 */
bool ibutton_protocols_write_copy(iButtonProtocols* protocols, iButtonKey* key);

/**
 * Start emulating the key
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] key pointer to the key to be emulated
 */
void ibutton_protocols_emulate_start(iButtonProtocols* protocols, iButtonKey* key);

/**
 * Stop emulating the key
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] key pointer to the key to be emulated
 */
void ibutton_protocols_emulate_stop(iButtonProtocols* protocols, iButtonKey* key);

/**
 * Save the key data to a file.
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] key pointer to the key to be saved
 * @param [in] file_name full absolute path to the file name
 * @return true on success, false on failure
 */
bool ibutton_protocols_save(
    iButtonProtocols* protocols,
    const iButtonKey* key,
    const char* file_name);

/**
 * Load the key from a file.
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [out] key pointer to the key to load into (must be allocated before)
 * @param [in] file_name full absolute path to the file name
 * @return true on success, false on failure
 */
bool ibutton_protocols_load(iButtonProtocols* protocols, iButtonKey* key, const char* file_name);

/**
 * Format a string containing device full data
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] key pointer to the key to be rendered
 * @param [out] result pointer to the FuriString instance (must be initialized)
 */
void ibutton_protocols_render_data(
    iButtonProtocols* protocols,
    const iButtonKey* key,
    FuriString* result);

/**
 * Format a string containing device brief data
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] key pointer to the key to be rendered
 * @param [out] result pointer to the FuriString instance (must be initialized)
 */
void ibutton_protocols_render_brief_data(
    iButtonProtocols* protocols,
    const iButtonKey* key,
    FuriString* result);

/**
 * Format a string containing error message (for invalid keys)
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] key pointer to the key to be rendered
 * @param [out] result pointer to the FuriString instance (must be initialized)
 */
void ibutton_protocols_render_error(
    iButtonProtocols* protocols,
    const iButtonKey* key,
    FuriString* result);

/**
 * Check whether the key data is valid
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] key pointer to the key to be checked
 * @return true if data is valid, false otherwise
 */
bool ibutton_protocols_is_valid(iButtonProtocols* protocols, const iButtonKey* key);

/**
 * Get a pointer to the key's editable data (for in-place editing)
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in] key pointer to the key to be checked
 * @param [out] editable pointer to a structure to contain the editable data
 */
void ibutton_protocols_get_editable_data(
    iButtonProtocols* protocols,
    const iButtonKey* key,
    iButtonEditableData* editable);

/**
 * Make all necessary internal adjustments after editing the key
 * @param [in] protocols pointer to an iButtonProtocols object
 * @param [in,out] key pointer to the key to be adjusted
 */
void ibutton_protocols_apply_edits(iButtonProtocols* protocols, const iButtonKey* key);

#ifdef __cplusplus
}
#endif
