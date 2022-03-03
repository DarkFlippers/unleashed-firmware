#pragma once

#include <furi.h>

#include "subghz_keystore.h"

typedef struct SubGhzEnvironment SubGhzEnvironment;

SubGhzEnvironment* subghz_environment_alloc();

void subghz_environment_free(SubGhzEnvironment* instance);

bool subghz_environment_load_keystore(SubGhzEnvironment* instance, const char* filename);

SubGhzKeystore* subghz_environment_get_keystore(SubGhzEnvironment* instance);

void subghz_environment_set_came_atomo_rainbow_table_file_name(
    SubGhzEnvironment* instance,
    const char* filename);

const char* subghz_environment_get_came_atomo_rainbow_table_file_name(SubGhzEnvironment* instance);

void subghz_environment_set_nice_flor_s_rainbow_table_file_name(
    SubGhzEnvironment* instance,
    const char* filename);

const char*
    subghz_environment_get_nice_flor_s_rainbow_table_file_name(SubGhzEnvironment* instance);
