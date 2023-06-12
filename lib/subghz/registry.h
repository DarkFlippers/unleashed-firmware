#pragma once

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SubGhzEnvironment SubGhzEnvironment;

typedef struct SubGhzProtocolRegistry SubGhzProtocolRegistry;
typedef struct SubGhzProtocol SubGhzProtocol;

struct SubGhzProtocolRegistry {
    const SubGhzProtocol** items;
    const size_t size;
};

/**
 * Registration by name SubGhzProtocol.
 * @param protocol_registry SubGhzProtocolRegistry
 * @param name Protocol name
 * @return SubGhzProtocol* pointer to a SubGhzProtocol instance
 */
const SubGhzProtocol* subghz_protocol_registry_get_by_name(
    const SubGhzProtocolRegistry* protocol_registry,
    const char* name);

/**
 * Registration protocol by index in array SubGhzProtocol.
 * @param protocol_registry SubGhzProtocolRegistry
 * @param index Protocol by index in array
 * @return SubGhzProtocol* pointer to a SubGhzProtocol instance
 */
const SubGhzProtocol* subghz_protocol_registry_get_by_index(
    const SubGhzProtocolRegistry* protocol_registry,
    size_t index);

/**
 * Getting the number of registered protocols.
 * @param protocol_registry SubGhzProtocolRegistry
 * @return Number of protocols
 */
size_t subghz_protocol_registry_count(const SubGhzProtocolRegistry* protocol_registry);

#ifdef __cplusplus
}
#endif
