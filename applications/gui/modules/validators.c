#include <furi.h>
#include "validators.h"
#include "applications/storage/storage.h"

struct ValidatorIsFile {
    char* app_path_folder;
    const char* app_extension;
    char* current_name;
};

bool validator_is_file_callback(const char* text, string_t error, void* context) {
    furi_assert(context);
    ValidatorIsFile* instance = context;

    if(instance->current_name != NULL) {
        if(strcmp(instance->current_name, text) == 0) {
            return true;
        }
    }

    bool ret = true;
    string_t path;
    string_init_printf(path, "%s/%s%s", instance->app_path_folder, text, instance->app_extension);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    if(storage_common_stat(storage, string_get_cstr(path), NULL) == FSE_OK) {
        ret = false;
        string_printf(error, "This name\nexists!\nChoose\nanother one.");
    } else {
        ret = true;
    }
    string_clear(path);
    furi_record_close(RECORD_STORAGE);

    return ret;
}

ValidatorIsFile* validator_is_file_alloc_init(
    const char* app_path_folder,
    const char* app_extension,
    const char* current_name) {
    ValidatorIsFile* instance = malloc(sizeof(ValidatorIsFile));

    instance->app_path_folder = strdup(app_path_folder);
    instance->app_extension = app_extension;
    instance->current_name = strdup(current_name);

    return instance;
}

void validator_is_file_free(ValidatorIsFile* instance) {
    furi_assert(instance);
    free(instance->app_path_folder);
    free(instance->current_name);
    free(instance);
}
