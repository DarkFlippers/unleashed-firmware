#include "input_settings.h"
#include "input_settings_filename.h"
#include "input.h"

#include <saved_struct.h>
#include <storage/storage.h>

#define TAG "InputSettings"

#define INPUT_SETTINGS_VER (2) // version number

#define INPUT_SETTINGS_PATH  INT_PATH(INPUT_SETTINGS_FILE_NAME)
#define INPUT_SETTINGS_MAGIC (0x29)

#define INPUT_SETTINGS_VIBRO_TOUCH_TRIGGER_MASK_DEFAULT \
    ((1 << InputTypePress) | (1 << InputTypeRelease))

void input_settings_load(InputSettings* settings) {
    furi_assert(settings);

    bool success = false;

    //a useless cycle do-while, may will be used in future with anoter condition
    do {
        // take version from settings file metadata, if cant then break and fill settings with 0 and save to settings file;
        uint8_t version;
        if(!saved_struct_get_metadata(INPUT_SETTINGS_PATH, NULL, &version, NULL)) break;

        if(version == 1) {
            struct {
                uint8_t vibro_touch_level;
            } v1;
            if(!saved_struct_load(INPUT_SETTINGS_PATH, &v1, sizeof(v1), INPUT_SETTINGS_MAGIC, 1))
                break;
            settings->vibro_touch_level = v1.vibro_touch_level;
            settings->vibro_touch_trigger_mask = INPUT_SETTINGS_VIBRO_TOUCH_TRIGGER_MASK_DEFAULT;
            success = true;
            break;
        }

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
        settings->vibro_touch_trigger_mask = INPUT_SETTINGS_VIBRO_TOUCH_TRIGGER_MASK_DEFAULT;
        //input_settings_save(settings);
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
