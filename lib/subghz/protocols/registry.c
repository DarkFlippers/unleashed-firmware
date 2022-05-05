#include "registry.h"

const SubGhzProtocol* subghz_protocol_registry[] = {
    &subghz_protocol_princeton,    &subghz_protocol_keeloq,     &subghz_protocol_star_line,
    &subghz_protocol_nice_flo,     &subghz_protocol_came,       &subghz_protocol_faac_slh,
    &subghz_protocol_nice_flor_s,  &subghz_protocol_came_twee,  &subghz_protocol_came_atomo,
    &subghz_protocol_nero_sketch,  &subghz_protocol_ido,        &subghz_protocol_kia,
    &subghz_protocol_hormann,      &subghz_protocol_nero_radio, &subghz_protocol_somfy_telis,
    &subghz_protocol_somfy_keytis, &subghz_protocol_scher_khan, &subghz_protocol_gate_tx,
    &subghz_protocol_raw,          &subghz_protocol_firefly,    &subghz_protocol_secplus_v2,
    &subghz_protocol_secplus_v1,

};

const SubGhzProtocol* subghz_protocol_registry_get_by_name(const char* name) {
    for(size_t i = 0; i < subghz_protocol_registry_count(); i++) {
        if(strcmp(name, subghz_protocol_registry[i]->name) == 0) {
            return subghz_protocol_registry[i];
        }
    }
    return NULL;
}

const SubGhzProtocol* subghz_protocol_registry_get_by_index(size_t index) {
    if(index < subghz_protocol_registry_count()) {
        return subghz_protocol_registry[index];
    } else {
        return NULL;
    }
}

size_t subghz_protocol_registry_count() {
    return COUNT_OF(subghz_protocol_registry);
}
