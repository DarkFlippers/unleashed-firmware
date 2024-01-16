#include "expansion_settings.h"

#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#include "expansion_settings_filename.h"

#define EXPANSION_SETTINGS_PATH INT_PATH(EXPANSION_SETTINGS_FILE_NAME)
#define EXPANSION_SETTINGS_VERSION (0)
#define EXPANSION_SETTINGS_MAGIC (0xEA)

bool expansion_settings_load(ExpansionSettings* settings) {
    furi_assert(settings);
    return saved_struct_load(
        EXPANSION_SETTINGS_PATH,
        settings,
        sizeof(ExpansionSettings),
        EXPANSION_SETTINGS_MAGIC,
        EXPANSION_SETTINGS_VERSION);
}

bool expansion_settings_save(ExpansionSettings* settings) {
    furi_assert(settings);
    return saved_struct_save(
        EXPANSION_SETTINGS_PATH,
        settings,
        sizeof(ExpansionSettings),
        EXPANSION_SETTINGS_MAGIC,
        EXPANSION_SETTINGS_VERSION);
}
