#pragma once

#include "../types.h"

/**
 * Registration by name SubGhzProtocol.
 * @param name Protocol name
 * @return SubGhzProtocol* pointer to a SubGhzProtocol instance
 */
const SubGhzProtocol* subghz_protocol_registry_get_by_name(const char* name);

/**
 * Registration protocol by index in array SubGhzProtocol.
 * @param index Protocol by index in array
 * @return SubGhzProtocol* pointer to a SubGhzProtocol instance
 */
const SubGhzProtocol* subghz_protocol_registry_get_by_index(size_t index);

/**
 * Getting the number of registered protocols.
 * @return Number of protocols
 */
size_t subghz_protocol_registry_count();
