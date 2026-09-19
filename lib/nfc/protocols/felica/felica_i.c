#include "felica_i.h"

void felica_system_init(FelicaSystem* system) {
    system->system_code = 0;
    system->system_code_idx = 0;
    system->key_version = 0;
    system->services = simple_array_alloc(&felica_service_array_cfg);
    system->areas = simple_array_alloc(&felica_area_array_cfg);
    system->public_blocks = simple_array_alloc(&felica_public_block_array_cfg);
}

void felica_system_reset(FelicaSystem* system) {
    furi_check(system);
    system->system_code = 0;
    system->system_code_idx = 0;
    furi_check(system->services);
    furi_check(system->areas);
    furi_check(system->public_blocks);
    simple_array_free(system->services);
    simple_array_free(system->areas);
    simple_array_free(system->public_blocks);
    memset(system, 0, sizeof(FelicaSystem));
}

void felica_system_copy(FelicaSystem* system, const FelicaSystem* other) {
    furi_check(system);
    furi_check(other);
    system->system_code = other->system_code;
    system->system_code_idx = other->system_code_idx;
    system->key_version = other->key_version;
    simple_array_copy(system->services, other->services);
    simple_array_copy(system->areas, other->areas);
    simple_array_copy(system->public_blocks, other->public_blocks);
}

static bool felica_system_is_equal(const FelicaSystem* system, const FelicaSystem* other) {
    return system->system_code == other->system_code &&
           system->system_code_idx == other->system_code_idx &&
           system->key_version == other->key_version &&
           simple_array_is_equal(system->services, other->services) &&
           simple_array_is_equal(system->areas, other->areas) &&
           simple_array_is_equal(system->public_blocks, other->public_blocks);
}

// A system owns its nested arrays, so a bytewise compare would only compare their pointers
bool felica_system_array_is_equal(const SimpleArray* instance, const SimpleArray* other) {
    furi_check(instance);
    furi_check(other);

    const uint32_t count = simple_array_get_count(instance);
    if(count != simple_array_get_count(other)) return false;

    for(uint32_t i = 0; i < count; i++) {
        if(!felica_system_is_equal(simple_array_cget(instance, i), simple_array_cget(other, i)))
            return false;
    }

    return true;
}

const SimpleArrayConfig felica_service_array_cfg = {
    .init = NULL,
    .copy = NULL,
    .reset = NULL,
    .type_size = sizeof(FelicaService),
};

const SimpleArrayConfig felica_area_array_cfg = {
    .init = NULL,
    .copy = NULL,
    .reset = NULL,
    .type_size = sizeof(FelicaArea),
};

const SimpleArrayConfig felica_public_block_array_cfg = {
    .init = NULL,
    .copy = NULL,
    .reset = NULL,
    .type_size = sizeof(FelicaPublicBlock),
};

const SimpleArrayConfig felica_system_array_cfg = {
    .init = (SimpleArrayInit)felica_system_init,
    .copy = (SimpleArrayCopy)felica_system_copy,
    .reset = (SimpleArrayReset)felica_system_reset,
    .type_size = sizeof(FelicaSystem),
};
