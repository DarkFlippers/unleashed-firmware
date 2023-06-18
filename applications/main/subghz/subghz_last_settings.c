#include "subghz_last_settings.h"
#include "subghz_i.h"

#define TAG "SubGhzLastSettings"

#define SUBGHZ_LAST_SETTING_FILE_TYPE "Flipper SubGhz Last Setting File"
#define SUBGHZ_LAST_SETTING_FILE_VERSION 1
#define SUBGHZ_LAST_SETTINGS_PATH EXT_PATH("subghz/assets/last_subghz.settings")
// 1 = "AM650"
// "AM270", "AM650", "FM238", "FM476",
#define SUBGHZ_LAST_SETTING_DEFAULT_PRESET 1
#define SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY 433920000
#define SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_FEEDBACK_LEVEL 2
#define SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_TRIGGER -93.0f

#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY "Frequency"
//#define SUBGHZ_LAST_SETTING_FIELD_PRESET "Preset"
#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_FEEDBACK_LEVEL "FeedbackLevel"
#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_TRIGGER "FATrigger"
#define SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_ENABLED "External"
#define SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_POWER "ExtPower"
#define SUBGHZ_LAST_SETTING_FIELD_TIMESTAMP_FILE_NAMES "TimestampNames"

SubGhzLastSettings* subghz_last_settings_alloc(void) {
    SubGhzLastSettings* instance = malloc(sizeof(SubGhzLastSettings));
    return instance;
}

void subghz_last_settings_free(SubGhzLastSettings* instance) {
    furi_assert(instance);
    free(instance);
}

void subghz_last_settings_load(SubGhzLastSettings* instance, size_t preset_count) {
    UNUSED(preset_count);
    furi_assert(instance);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "subghz_last_settings_load");
#endif

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    uint32_t temp_frequency = 0;
    uint32_t temp_frequency_analyzer_feedback_level = 0;
    float temp_frequency_analyzer_trigger = 0;
    bool temp_external_module_enabled = false;
    bool temp_external_module_power_5v_disable = false;
    bool temp_timestamp_file_names = false;
    //int32_t temp_preset = 0;
    bool frequency_analyzer_feedback_level_was_read = false;
    bool frequency_analyzer_trigger_was_read = false;

    if(FSE_OK == storage_sd_status(storage) && SUBGHZ_LAST_SETTINGS_PATH &&
       flipper_format_file_open_existing(fff_data_file, SUBGHZ_LAST_SETTINGS_PATH)) {
        /*
        flipper_format_read_int32(
            fff_data_file, SUBGHZ_LAST_SETTING_FIELD_PRESET, (int32_t*)&temp_preset, 1);*/
        flipper_format_read_uint32(
            fff_data_file, SUBGHZ_LAST_SETTING_FIELD_FREQUENCY, (uint32_t*)&temp_frequency, 1);
        frequency_analyzer_feedback_level_was_read = flipper_format_read_uint32(
            fff_data_file,
            SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_FEEDBACK_LEVEL,
            (uint32_t*)&temp_frequency_analyzer_feedback_level,
            1);
        frequency_analyzer_trigger_was_read = flipper_format_read_float(
            fff_data_file,
            SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_TRIGGER,
            (float*)&temp_frequency_analyzer_trigger,
            1);
        flipper_format_read_bool(
            fff_data_file,
            SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_ENABLED,
            (bool*)&temp_external_module_enabled,
            1);
        flipper_format_read_bool(
            fff_data_file,
            SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_POWER,
            (bool*)&temp_external_module_power_5v_disable,
            1);
        flipper_format_read_bool(
            fff_data_file,
            SUBGHZ_LAST_SETTING_FIELD_TIMESTAMP_FILE_NAMES,
            (bool*)&temp_timestamp_file_names,
            1);

    } else {
        FURI_LOG_E(TAG, "Error open file %s", SUBGHZ_LAST_SETTINGS_PATH);
    }

    if(temp_frequency == 0 || !furi_hal_subghz_is_tx_allowed(temp_frequency)) {
        FURI_LOG_W(TAG, "Last used frequency not found or can't be used!");
        instance->frequency = SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY;
        instance->preset = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;
        instance->frequency_analyzer_feedback_level =
            SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_FEEDBACK_LEVEL;
        instance->frequency_analyzer_trigger = SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_TRIGGER;
        instance->external_module_enabled = false;
        instance->timestamp_file_names = false;

    } else {
        instance->frequency = temp_frequency;
        instance->frequency_analyzer_feedback_level =
            frequency_analyzer_feedback_level_was_read ?
                temp_frequency_analyzer_feedback_level :
                SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_FEEDBACK_LEVEL;

        instance->frequency_analyzer_trigger = frequency_analyzer_trigger_was_read ?
                                                   temp_frequency_analyzer_trigger :
                                                   SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_TRIGGER;

        /*if(temp_preset > (int32_t)preset_count - 1 || temp_preset < 0) {
            FURI_LOG_W(TAG, "Last used preset no found");*/
        instance->preset = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;

        instance->external_module_enabled = temp_external_module_enabled;

        instance->external_module_power_5v_disable = temp_external_module_power_5v_disable;

        instance->timestamp_file_names = temp_timestamp_file_names;

        /*/} else {
            instance->preset = temp_preset;
        }*/
    }

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);
}

bool subghz_last_settings_save(SubGhzLastSettings* instance) {
    furi_assert(instance);
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "last_settings_save");
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

        /*
        if(!flipper_format_insert_or_update_int32(
               file, SUBGHZ_LAST_SETTING_FIELD_PRESET, &instance->preset, 1)) {
            break;
        }*/
        if(!flipper_format_insert_or_update_uint32(
               file, SUBGHZ_LAST_SETTING_FIELD_FREQUENCY, &instance->frequency, 1)) {
            break;
        }
        if(!flipper_format_insert_or_update_uint32(
               file,
               SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_FEEDBACK_LEVEL,
               &instance->frequency_analyzer_feedback_level,
               1)) {
            break;
        }
        if(!flipper_format_insert_or_update_float(
               file,
               SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_TRIGGER,
               &instance->frequency_analyzer_trigger,
               1)) {
            break;
        }
        if(!flipper_format_insert_or_update_bool(
               file,
               SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_ENABLED,
               &instance->external_module_enabled,
               1)) {
            break;
        }
        if(!flipper_format_insert_or_update_bool(
               file,
               SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_POWER,
               &instance->external_module_power_5v_disable,
               1)) {
            break;
        }
        if(!flipper_format_insert_or_update_bool(
               file,
               SUBGHZ_LAST_SETTING_FIELD_TIMESTAMP_FILE_NAMES,
               &instance->timestamp_file_names,
               1)) {
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
