#include "registry.h"

const SubGhzProtocol* subghz_protocol_registry_get_by_name(
    const SubGhzProtocolRegistry* protocol_registry,
    const char* name) {
    furi_check(protocol_registry);

    for(size_t i = 0; i < subghz_protocol_registry_count(protocol_registry); i++) {
        if(strcmp(name, protocol_registry->items[i]->name) == 0) {
            return protocol_registry->items[i];
        }
    }
    return NULL;
}

const SubGhzProtocol* subghz_protocol_registry_get_by_index(
    const SubGhzProtocolRegistry* protocol_registry,
    size_t index) {
    furi_check(protocol_registry);
    if(index < subghz_protocol_registry_count(protocol_registry)) {
        return protocol_registry->items[index];
    } else {
        return NULL;
    }
}

size_t subghz_protocol_registry_count(const SubGhzProtocolRegistry* protocol_registry) {
    furi_check(protocol_registry);
    return protocol_registry->size;
}
