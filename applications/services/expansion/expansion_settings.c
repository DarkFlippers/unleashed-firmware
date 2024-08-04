#include "expansion_settings.h"

#include <storage/storage.h>
#include <toolbox/saved_struct.h>

#include "expansion_settings_filename.h"

#define TAG "ExpansionSettings"

#define EXPANSION_SETTINGS_PATH    INT_PATH(EXPANSION_SETTINGS_FILE_NAME)
#define EXPANSION_SETTINGS_VERSION (0)
#define EXPANSION_SETTINGS_MAGIC   (0xEA)

void expansion_settings_load(ExpansionSettings* settings) {
    furi_assert(settings);

    const bool success = saved_struct_load(
        EXPANSION_SETTINGS_PATH,
        settings,
        sizeof(ExpansionSettings),
        EXPANSION_SETTINGS_MAGIC,
        EXPANSION_SETTINGS_VERSION);

    if(!success) {
        FURI_LOG_W(TAG, "Failed to load file, using defaults");
        memset(settings, 0, sizeof(ExpansionSettings));
        expansion_settings_save(settings);
    }
}

void expansion_settings_save(const ExpansionSettings* settings) {
    furi_assert(settings);

    const bool success = saved_struct_save(
        EXPANSION_SETTINGS_PATH,
        settings,
        sizeof(ExpansionSettings),
        EXPANSION_SETTINGS_MAGIC,
        EXPANSION_SETTINGS_VERSION);

    if(!success) {
        FURI_LOG_E(TAG, "Failed to save file");
    }
}
