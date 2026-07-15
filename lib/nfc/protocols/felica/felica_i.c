#include "felica_i.h"

void felica_system_init(FelicaSystem* system) {
    system->system_code = 0;
    system->system_code_idx = 0;
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
    simple_array_copy(system->services, other->services);
    simple_array_copy(system->areas, other->areas);
    simple_array_copy(system->public_blocks, other->public_blocks);
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
