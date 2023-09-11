#include "subghz_last_settings.h"
#include "subghz_i.h"

#define TAG "SubGhzLastSettings"

#define SUBGHZ_LAST_SETTING_FILE_TYPE "Flipper SubGhz Last Setting File"
#define SUBGHZ_LAST_SETTING_FILE_VERSION 1
#define SUBGHZ_LAST_SETTINGS_PATH EXT_PATH("subghz/assets/last_subghz.settings")

#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY "Frequency"
#define SUBGHZ_LAST_SETTING_FIELD_PRESET "Preset" // AKA Modulation
#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_FEEDBACK_LEVEL "FeedbackLevel"
#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_TRIGGER "FATrigger"
#define SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_ENABLED "External"
#define SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_POWER "ExtPower"
#define SUBGHZ_LAST_SETTING_FIELD_TIMESTAMP_FILE_NAMES "TimestampNames"
#define SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_POWER_AMP "ExtPowerAmp"
#define SUBGHZ_LAST_SETTING_FIELD_HOPPING_ENABLE "Hopping"
#define SUBGHZ_LAST_SETTING_FIELD_IGNORE_FILTER "IgnoreFilter"
#define SUBGHZ_LAST_SETTING_FIELD_FILTER "Filter"
#define SUBGHZ_LAST_SETTING_FIELD_RSSI_THRESHOLD "RSSI"

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

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    uint32_t temp_frequency = 0;
    uint32_t temp_frequency_analyzer_feedback_level = 0;
    float temp_frequency_analyzer_trigger = 0;
    bool temp_external_module_enabled = false;
    bool temp_external_module_power_5v_disable = false;
    bool temp_external_module_power_amp = false;
    bool temp_timestamp_file_names = false;
    bool temp_enable_hopping = false;
    uint32_t temp_ignore_filter = 0;
    uint32_t temp_filter = 0;
    float temp_rssi = 0;
    uint32_t temp_preset = 0;

    bool preset_was_read = false;
    bool rssi_was_read = false;
    bool filter_was_read = false;
    bool ignore_filter_was_read = false;
    bool frequency_analyzer_feedback_level_was_read = false;
    bool frequency_analyzer_trigger_was_read = false;

    if(FSE_OK == storage_sd_status(storage) && SUBGHZ_LAST_SETTINGS_PATH &&
       flipper_format_file_open_existing(fff_data_file, SUBGHZ_LAST_SETTINGS_PATH)) {
        preset_was_read = flipper_format_read_uint32(
            fff_data_file, SUBGHZ_LAST_SETTING_FIELD_PRESET, (uint32_t*)&temp_preset, 1);
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
        flipper_format_read_bool(
            fff_data_file,
            SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_POWER_AMP,
            (bool*)&temp_external_module_power_amp,
            1);
        flipper_format_read_bool(
            fff_data_file,
            SUBGHZ_LAST_SETTING_FIELD_HOPPING_ENABLE,
            (bool*)&temp_enable_hopping,
            1);
        rssi_was_read = flipper_format_read_float(
            fff_data_file, SUBGHZ_LAST_SETTING_FIELD_RSSI_THRESHOLD, (float*)&temp_rssi, 1);
        ignore_filter_was_read = flipper_format_read_uint32(
            fff_data_file,
            SUBGHZ_LAST_SETTING_FIELD_IGNORE_FILTER,
            (uint32_t*)&temp_ignore_filter,
            1);
        filter_was_read = flipper_format_read_uint32(
            fff_data_file, SUBGHZ_LAST_SETTING_FIELD_FILTER, (uint32_t*)&temp_filter, 1);
    } else {
        FURI_LOG_E(TAG, "Error open file %s", SUBGHZ_LAST_SETTINGS_PATH);
    }

    if(temp_frequency == 0 || !furi_hal_subghz_is_tx_allowed(temp_frequency)) {
        FURI_LOG_W(TAG, "Last used frequency not found or can't be used!");

        instance->frequency = SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY;
        instance->preset_index = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;
        instance->frequency_analyzer_feedback_level =
            SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_FEEDBACK_LEVEL;
        instance->frequency_analyzer_trigger = SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_TRIGGER;
        instance->external_module_enabled = false;
        instance->timestamp_file_names = false;
        instance->external_module_power_amp = false;
        instance->enable_hopping = false;
        instance->ignore_filter = 0x00;
        // See bin_raw_value in applications/main/subghz/scenes/subghz_scene_receiver_config.c
        instance->filter = SubGhzProtocolFlag_Decodable;
        instance->rssi = SUBGHZ_RAW_THRESHOLD_MIN;
    } else {
        instance->frequency = temp_frequency;
        instance->frequency_analyzer_feedback_level =
            frequency_analyzer_feedback_level_was_read ?
                temp_frequency_analyzer_feedback_level :
                SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_FEEDBACK_LEVEL;

        instance->frequency_analyzer_trigger = frequency_analyzer_trigger_was_read ?
                                                   temp_frequency_analyzer_trigger :
                                                   SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_TRIGGER;

        if(!preset_was_read) {
            FURI_LOG_W(TAG, "Preset was not read. Set default");
            instance->preset_index = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;
        } else if(temp_preset > (uint32_t)preset_count - 1) {
            FURI_LOG_W(
                TAG,
                "Last used preset out of range. Preset to set: %ld, Max index: %ld. Set default",
                temp_preset,
                (uint32_t)preset_count - 1);
            instance->preset_index = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;
        } else {
            instance->preset_index = temp_preset;
        }
        instance->external_module_enabled = temp_external_module_enabled;

        instance->external_module_power_5v_disable = temp_external_module_power_5v_disable;

        instance->timestamp_file_names = temp_timestamp_file_names;

        // External power amp CC1101
        instance->external_module_power_amp = temp_external_module_power_amp;

        instance->rssi = rssi_was_read ? temp_rssi : SUBGHZ_RAW_THRESHOLD_MIN;
        instance->enable_hopping = temp_enable_hopping;
        instance->ignore_filter = ignore_filter_was_read ? temp_ignore_filter : 0x00;
#if SUBGHZ_LAST_SETTING_SAVE_BIN_RAW
        instance->filter = filter_was_read ? temp_filter : SubGhzProtocolFlag_Decodable;
#else
        if(filter_was_read) {
            instance->filter = temp_filter != SubGhzProtocolFlag_Decodable ?
                                   SubGhzProtocolFlag_Decodable :
                                   temp_filter;
        } else {
            instance->filter = SubGhzProtocolFlag_Decodable;
        }
#endif
        // Set globally in furi hal
        furi_hal_subghz_set_ext_power_amp(instance->external_module_power_amp);
    }

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);
}

bool subghz_last_settings_save(SubGhzLastSettings* instance) {
    furi_assert(instance);

#if SUBGHZ_LAST_SETTING_SAVE_BIN_RAW != true
    instance->filter = SubGhzProtocolFlag_Decodable;
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
        if(!flipper_format_insert_or_update_uint32(
               file, SUBGHZ_LAST_SETTING_FIELD_PRESET, &instance->preset_index, 1)) {
            break;
        }
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
        if(!flipper_format_insert_or_update_bool(
               file,
               SUBGHZ_LAST_SETTING_FIELD_EXTERNAL_MODULE_POWER_AMP,
               &instance->external_module_power_amp,
               1)) {
            break;
        }
        if(!flipper_format_insert_or_update_bool(
               file, SUBGHZ_LAST_SETTING_FIELD_HOPPING_ENABLE, &instance->enable_hopping, 1)) {
            break;
        }
        if(!flipper_format_insert_or_update_float(
               file, SUBGHZ_LAST_SETTING_FIELD_RSSI_THRESHOLD, &instance->rssi, 1)) {
            break;
        }
        if(!flipper_format_insert_or_update_uint32(
               file, SUBGHZ_LAST_SETTING_FIELD_IGNORE_FILTER, &instance->ignore_filter, 1)) {
            break;
        }
        if(!flipper_format_insert_or_update_uint32(
               file, SUBGHZ_LAST_SETTING_FIELD_FILTER, &instance->filter, 1)) {
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

const char* LOG_ON = "ON";
const char* LOG_OFF = "OFF";

static inline const char*
    subghz_last_settings_log_filter_get_index(uint32_t filter, uint32_t flag) {
    return READ_BIT(filter, flag) > 0 ? LOG_ON : LOG_OFF;
}

static inline const char* bool_to_char(bool value) {
    return value ? LOG_ON : LOG_OFF;
}

void subghz_last_settings_log(SubGhzLastSettings* instance) {
    furi_assert(instance);

    FURI_LOG_I(
        TAG,
        "Frequency: %03ld.%02ld, FeedbackLevel: %ld, FATrigger: %.2f, External: %s, ExtPower: %s, TimestampNames: %s, ExtPowerAmp: %s,\n"
        "Hopping: %s,\nPreset: %ld, RSSI: %.2f, "
        "Starline: %s, Cars: %s, Magellan: %s, BinRAW: %s",
        instance->frequency / 1000000 % 1000,
        instance->frequency / 10000 % 100,
        instance->frequency_analyzer_feedback_level,
        (double)instance->frequency_analyzer_trigger,
        bool_to_char(instance->external_module_enabled),
        bool_to_char(instance->external_module_power_5v_disable),
        bool_to_char(instance->timestamp_file_names),
        bool_to_char(instance->external_module_power_amp),
        bool_to_char(instance->enable_hopping),
        instance->preset_index,
        (double)instance->rssi,
        subghz_last_settings_log_filter_get_index(
            instance->ignore_filter, SubGhzProtocolFlag_StarLine),
        subghz_last_settings_log_filter_get_index(
            instance->ignore_filter, SubGhzProtocolFlag_AutoAlarms),
        subghz_last_settings_log_filter_get_index(
            instance->ignore_filter, SubGhzProtocolFlag_Magellan),
        subghz_last_settings_log_filter_get_index(instance->filter, SubGhzProtocolFlag_BinRAW));
}
