#include "input_settings.h"
#include "input_settings_filename.h"

#include <saved_struct.h>
#include <storage/storage.h>

#define TAG "InputSettings"

#define INPUT_SETTINGS_VER (1) // version nnumber

#define INPUT_SETTINGS_PATH  INT_PATH(INPUT_SETTINGS_FILE_NAME)
#define INPUT_SETTINGS_MAGIC (0x29)

void input_settings_load(InputSettings* settings) {
    furi_assert(settings);

    bool success = false;

    //a useless cycle do-while, may will be used in future with anoter condition
    do {
        // take version from settings file metadata, if cant then break and fill settings with 0 and save to settings file;
        uint8_t version;
        if(!saved_struct_get_metadata(INPUT_SETTINGS_PATH, NULL, &version, NULL)) break;

        // if config actual version - load it directly
        if(version == INPUT_SETTINGS_VER) {
            success = saved_struct_load(
                INPUT_SETTINGS_PATH,
                settings,
                sizeof(InputSettings),
                INPUT_SETTINGS_MAGIC,
                INPUT_SETTINGS_VER);
        }
        // in case of another config version we exit from useless cycle to next step
    } while(false);

    // fill settings with 0 and save to settings file;
    if(!success) {
        FURI_LOG_W(TAG, "Failed to load file, using defaults");
        memset(settings, 0, sizeof(InputSettings));
        input_settings_save(settings);
    }
}

void input_settings_save(const InputSettings* settings) {
    furi_assert(settings);

    const bool success = saved_struct_save(
        INPUT_SETTINGS_PATH,
        settings,
        sizeof(InputSettings),
        INPUT_SETTINGS_MAGIC,
        INPUT_SETTINGS_VER);

    if(!success) {
        FURI_LOG_E(TAG, "Failed to save file");
    }
}
