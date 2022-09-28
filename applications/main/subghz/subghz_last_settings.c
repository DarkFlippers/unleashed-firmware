#include "subghz_last_settings.h"
#include <lib/flipper_format/flipper_format.h>

#define TAG "SubGhzLastSettings"

#define SUBGHZ_LAST_SETTING_FILE_TYPE "Flipper SubGhz Last Setting File"
#define SUBGHZ_LAST_SETTING_FILE_VERSION 1
#define SUBGHZ_LAST_SETTINGS_PATH EXT_PATH("subghz/assets/last_subghz.settings")
// 1 = "AM650"
// "AM270", "AM650", "FM238", "FM476",
#define SUBGHZ_LAST_SETTING_DEFAULT_PRESET 1
#define SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY 433920000

SubGhzLastSettings* subghz_last_settings_alloc(void) {
    SubGhzLastSettings* instance = malloc(sizeof(SubGhzLastSettings));
    return instance;
}

void subghz_last_settings_free(SubGhzLastSettings* instance) {
    furi_assert(instance);
    free(instance);
}

void subghz_last_settings_load(SubGhzLastSettings* instance, size_t preset_count) {
    furi_assert(instance);
#if FURI_DEBUG
    FURI_LOG_I(TAG, "subghz_last_settings_load");
#endif

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    uint32_t temp_frequency = 0;
    int32_t temp_preset = 0;

    if(FSE_OK == storage_sd_status(storage) && SUBGHZ_LAST_SETTINGS_PATH &&
       flipper_format_file_open_existing(fff_data_file, SUBGHZ_LAST_SETTINGS_PATH)) {
        flipper_format_read_int32(fff_data_file, "Preset", (int32_t*)&temp_preset, 1);
        flipper_format_read_uint32(fff_data_file, "Frequency", (uint32_t*)&temp_frequency, 1);
    } else {
        FURI_LOG_E(TAG, "Error open file %s", SUBGHZ_LAST_SETTINGS_PATH);
    }

    if(temp_frequency == 0 || !furi_hal_subghz_is_tx_allowed(temp_frequency)) {
        FURI_LOG_W(TAG, "Last used frequency not found or can't be used!");
        instance->frequency = SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY;
        instance->preset = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;
    } else {
        instance->frequency = temp_frequency;

        if(temp_preset > (int32_t)preset_count - 1 || temp_preset < 0) {
            FURI_LOG_W(TAG, "Last used preset no found");
            instance->preset = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;
        } else {
            instance->preset = temp_preset;
        }
    }

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);
}

bool subghz_last_settings_save(SubGhzLastSettings* instance) {
    furi_assert(instance);
#if FURI_DEBUG
    FURI_LOG_I(TAG, "subghz_last_settings_save");
#endif

    bool saved = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);

    do {
        if(FSE_OK != storage_sd_status(storage)) {
            break;
        }

        // Open file
        if(!flipper_format_file_open_always(file, SUBGHZ_LAST_SETTINGS_PATH)) break;

        // Write header
        if(!flipper_format_write_header_cstr(
               file, SUBGHZ_LAST_SETTING_FILE_TYPE, SUBGHZ_LAST_SETTING_FILE_VERSION))
            break;

        if(!flipper_format_insert_or_update_int32(file, "Preset", &instance->preset, 1)) {
            break;
        }
        if(!flipper_format_insert_or_update_uint32(file, "Frequency", &instance->frequency, 1)) {
            break;
        }
        saved = true;
    } while(0);

    if(!saved) {
        FURI_LOG_E(TAG, "Error save file %s", SUBGHZ_LAST_SETTINGS_PATH);
    }

    flipper_format_file_close(file);
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return saved;
}