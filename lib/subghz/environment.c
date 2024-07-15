#include "environment.h"
#include "registry.h"

struct SubGhzEnvironment {
    SubGhzKeystore* keystore;
    const SubGhzProtocolRegistry* protocol_registry;
    const char* came_atomo_rainbow_table_file_name;
    const char* nice_flor_s_rainbow_table_file_name;
    const char* alutech_at_4n_rainbow_table_file_name;
};

SubGhzEnvironment* subghz_environment_alloc(void) {
    SubGhzEnvironment* instance = malloc(sizeof(SubGhzEnvironment));

    instance->keystore = subghz_keystore_alloc();
    instance->protocol_registry = NULL;
    instance->came_atomo_rainbow_table_file_name = NULL;
    instance->nice_flor_s_rainbow_table_file_name = NULL;
    instance->alutech_at_4n_rainbow_table_file_name = NULL;

    return instance;
}

void subghz_environment_free(SubGhzEnvironment* instance) {
    furi_check(instance);

    instance->protocol_registry = NULL;
    instance->came_atomo_rainbow_table_file_name = NULL;
    instance->nice_flor_s_rainbow_table_file_name = NULL;
    instance->alutech_at_4n_rainbow_table_file_name = NULL;
    subghz_keystore_free(instance->keystore);

    free(instance);
}

bool subghz_environment_load_keystore(SubGhzEnvironment* instance, const char* filename) {
    furi_check(instance);

    return subghz_keystore_load(instance->keystore, filename);
}

SubGhzKeystore* subghz_environment_get_keystore(SubGhzEnvironment* instance) {
    furi_check(instance);

    return instance->keystore;
}

void subghz_environment_set_came_atomo_rainbow_table_file_name(
    SubGhzEnvironment* instance,
    const char* filename) {
    furi_check(instance);

    instance->came_atomo_rainbow_table_file_name = filename;
}

const char*
    subghz_environment_get_came_atomo_rainbow_table_file_name(SubGhzEnvironment* instance) {
    furi_check(instance);

    return instance->came_atomo_rainbow_table_file_name;
}

void subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
    SubGhzEnvironment* instance,
    const char* filename) {
    furi_check(instance);

    instance->alutech_at_4n_rainbow_table_file_name = filename;
}

const char*
    subghz_environment_get_alutech_at_4n_rainbow_table_file_name(SubGhzEnvironment* instance) {
    furi_check(instance);

    return instance->alutech_at_4n_rainbow_table_file_name;
}

void subghz_environment_set_nice_flor_s_rainbow_table_file_name(
    SubGhzEnvironment* instance,
    const char* filename) {
    furi_check(instance);

    instance->nice_flor_s_rainbow_table_file_name = filename;
}

const char*
    subghz_environment_get_nice_flor_s_rainbow_table_file_name(SubGhzEnvironment* instance) {
    furi_check(instance);

    return instance->nice_flor_s_rainbow_table_file_name;
}

void subghz_environment_set_protocol_registry(
    SubGhzEnvironment* instance,
    const SubGhzProtocolRegistry* protocol_registry_items) {
    furi_check(instance);
    const SubGhzProtocolRegistry* protocol_registry = protocol_registry_items;
    instance->protocol_registry = protocol_registry;
}

const SubGhzProtocolRegistry*
    subghz_environment_get_protocol_registry(SubGhzEnvironment* instance) {
    furi_check(instance);
    furi_check(instance->protocol_registry);
    return instance->protocol_registry;
}

const char*
    subghz_environment_get_protocol_name_registry(SubGhzEnvironment* instance, size_t idx) {
    furi_check(instance);
    furi_check(instance->protocol_registry);
    const SubGhzProtocol* protocol =
        subghz_protocol_registry_get_by_index(instance->protocol_registry, idx);
    if(protocol != NULL) {
        return protocol->name;
    } else {
        return NULL;
    }
}
