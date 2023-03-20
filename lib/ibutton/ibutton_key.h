/**
 * @file ibutton_key.h
 * 
 * iButton key data holder
 */

#pragma once

#include <core/string.h>

#include "protocols/protocol_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct iButtonKey iButtonKey;

/**
 * Allocate a key object
 * @param [in] data_size maximum data size held by the key
 * @return pointer to the key object
 */
iButtonKey* ibutton_key_alloc(size_t data_size);

/**
 * Destroy the key object, free resources
 * @param [in] key pointer to the key object
 */
void ibutton_key_free(iButtonKey* key);

/**
 * Get the protocol id held by the key
 * @param [in] key pointer to the key object
 * @return protocol id held by the key
 */
iButtonProtocolId ibutton_key_get_protocol_id(const iButtonKey* key);

/**
 * Set the protocol id held by the key
 * @param [in] key pointer to the key object
 * @param [in] protocol_id new protocol id
 */
void ibutton_key_set_protocol_id(iButtonKey* key, iButtonProtocolId protocol_id);

/**
 * Reset the protocol id and data held by the key
 * @param [in] key pointer to the key object
 */
void ibutton_key_reset(iButtonKey* key);

#ifdef __cplusplus
}
#endif
