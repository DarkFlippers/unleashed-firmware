#include "environment.h"

struct SubGhzEnvironment {
    SubGhzKeystore* keystore;
    const char* came_atomo_rainbow_table_file_name;
    const char* nice_flor_s_rainbow_table_file_name;
};

SubGhzEnvironment* subghz_environment_alloc() {
    SubGhzEnvironment* instance = malloc(sizeof(SubGhzEnvironment));

    instance->keystore = subghz_keystore_alloc();
    instance->came_atomo_rainbow_table_file_name = NULL;
    instance->nice_flor_s_rainbow_table_file_name = NULL;

    return instance;
}

void subghz_environment_free(SubGhzEnvironment* instance) {
    furi_assert(instance);

    subghz_keystore_free(instance->keystore);

    free(instance);
}

bool subghz_environment_load_keystore(SubGhzEnvironment* instance, const char* filename) {
    furi_assert(instance);

    return subghz_keystore_load(instance->keystore, filename);
}

SubGhzKeystore* subghz_environment_get_keystore(SubGhzEnvironment* instance) {
    furi_assert(instance);

    return instance->keystore;
}

void subghz_environment_set_came_atomo_rainbow_table_file_name(
    SubGhzEnvironment* instance,
    const char* filename) {
    furi_assert(instance);

    instance->came_atomo_rainbow_table_file_name = filename;
}

const char*
    subghz_environment_get_came_atomo_rainbow_table_file_name(SubGhzEnvironment* instance) {
    furi_assert(instance);

    return instance->came_atomo_rainbow_table_file_name;
}

void subghz_environment_set_nice_flor_s_rainbow_table_file_name(
    SubGhzEnvironment* instance,
    const char* filename) {
    furi_assert(instance);

    instance->nice_flor_s_rainbow_table_file_name = filename;
}

const char*
    subghz_environment_get_nice_flor_s_rainbow_table_file_name(SubGhzEnvironment* instance) {
    furi_assert(instance);

    return instance->nice_flor_s_rainbow_table_file_name;
}
