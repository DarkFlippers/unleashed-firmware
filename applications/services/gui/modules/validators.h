#pragma once

#include <core/common_defines.h>
#include <core/string.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct ValidatorIsFile ValidatorIsFile;

ValidatorIsFile* validator_is_file_alloc_init(
    const char* app_path_folder,
    const char* app_extension,
    const char* current_name);

void validator_is_file_free(ValidatorIsFile* instance);

bool validator_is_file_callback(const char* text, FuriString* error, void* context);

#ifdef __cplusplus
}
#endif
