#include "subghz_last_settings.h"
#include "subghz_i.h"

#define TAG "SubGhzLastSettings"

#define SUBGHZ_LAST_SETTING_FILE_TYPE    "Flipper SubGhz Last Setting File"
#define SUBGHZ_LAST_SETTING_FILE_VERSION 3
#define SUBGHZ_LAST_SETTINGS_PATH        EXT_PATH("subghz/assets/last_subghz.settings")

#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY                         "Frequency"
#define SUBGHZ_LAST_SETTING_FIELD_PRESET                            "Preset" // AKA Modulation
#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_FEEDBACK_LEVEL "FeedbackLevel"
#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_TRIGGER        "FATrigger"
#define SUBGHZ_LAST_SETTING_FIELD_PROTOCOL_FILE_NAMES               "ProtocolNames"
#define SUBGHZ_LAST_SETTING_FIELD_HOPPING_ENABLE                    "Hopping"
#define SUBGHZ_LAST_SETTING_FIELD_IGNORE_FILTER                     "IgnoreFilter"
#define SUBGHZ_LAST_SETTING_FIELD_FILTER                            "Filter"
#define SUBGHZ_LAST_SETTING_FIELD_RSSI_THRESHOLD                    "RSSI"
#define SUBGHZ_LAST_SETTING_FIELD_DELETE_OLD                        "DelOldSignals"
#define SUBGHZ_LAST_SETTING_FIELD_HOPPING_THRESHOLD                 "HoppingThreshold"

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

    // Default values (all others set to 0, if read from file fails these are used)
    instance->frequency = SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY;
    instance->preset_index = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;
    instance->frequency_analyzer_feedback_level =
        SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_FEEDBACK_LEVEL;
    instance->frequency_analyzer_trigger = SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_TRIGGER;
    // See bin_raw_value in scenes/subghz_scene_receiver_config.c
    instance->filter = SubGhzProtocolFlag_Decodable;
    instance->rssi = SUBGHZ_RAW_THRESHOLD_MIN;
    instance->hopping_threshold = -90.0f;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    FuriString* temp_str = furi_string_alloc();
    uint32_t config_version = 0;

    if(FSE_OK == storage_sd_status(storage) &&
       flipper_format_file_open_existing(fff_data_file, SUBGHZ_LAST_SETTINGS_PATH)) {
        do {
            if(!flipper_format_read_header(fff_data_file, temp_str, &config_version)) break;
            if((strcmp(furi_string_get_cstr(temp_str), SUBGHZ_LAST_SETTING_FILE_TYPE) != 0) ||
               (config_version != SUBGHZ_LAST_SETTING_FILE_VERSION)) {
                break;
            }

            if(!flipper_format_read_uint32(
                   fff_data_file, SUBGHZ_LAST_SETTING_FIELD_FREQUENCY, &instance->frequency, 1)) {
                flipper_format_rewind(fff_data_file);
            }
            if(!flipper_format_read_uint32(
                   fff_data_file, SUBGHZ_LAST_SETTING_FIELD_PRESET, &instance->preset_index, 1)) {
                flipper_format_rewind(fff_data_file);
            }
            if(!flipper_format_read_uint32(
                   fff_data_file,
                   SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_FEEDBACK_LEVEL,
                   &instance->frequency_analyzer_feedback_level,
                   1)) {
                flipper_format_rewind(fff_data_file);
            }
            if(!flipper_format_read_float(
                   fff_data_file,
                   SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_TRIGGER,
                   &instance->frequency_analyzer_trigger,
                   1)) {
                flipper_format_rewind(fff_data_file);
            }
            if(!flipper_format_read_bool(
                   fff_data_file,
                   SUBGHZ_LAST_SETTING_FIELD_PROTOCOL_FILE_NAMES,
                   &instance->protocol_file_names,
                   1)) {
                flipper_format_rewind(fff_data_file);
            }
            if(!flipper_format_read_bool(
                   fff_data_file,
                   SUBGHZ_LAST_SETTING_FIELD_HOPPING_ENABLE,
                   &instance->enable_hopping,
                   1)) {
                flipper_format_rewind(fff_data_file);
            }
            if(!flipper_format_read_uint32(
                   fff_data_file,
                   SUBGHZ_LAST_SETTING_FIELD_IGNORE_FILTER,
                   &instance->ignore_filter,
                   1)) {
                flipper_format_rewind(fff_data_file);
            }
            if(!flipper_format_read_uint32(
                   fff_data_file, SUBGHZ_LAST_SETTING_FIELD_FILTER, &instance->filter, 1)) {
                flipper_format_rewind(fff_data_file);
            }
            if(!flipper_format_read_float(
                   fff_data_file, SUBGHZ_LAST_SETTING_FIELD_RSSI_THRESHOLD, &instance->rssi, 1)) {
                flipper_format_rewind(fff_data_file);
            }
            if(!flipper_format_read_bool(
                   fff_data_file,
                   SUBGHZ_LAST_SETTING_FIELD_DELETE_OLD,
                   &instance->delete_old_signals,
                   1)) {
                flipper_format_rewind(fff_data_file);
            }
            if(!flipper_format_read_float(
                   fff_data_file,
                   SUBGHZ_LAST_SETTING_FIELD_HOPPING_THRESHOLD,
                   &instance->hopping_threshold,
                   1)) {
                flipper_format_rewind(fff_data_file);
            }

        } while(0);
    } else {
        FURI_LOG_E(TAG, "Error open file %s", SUBGHZ_LAST_SETTINGS_PATH);
    }

    furi_string_free(temp_str);

    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);

    if(instance->frequency == 0 || !furi_hal_subghz_is_tx_allowed(instance->frequency)) {
        instance->frequency = SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY;
    }

    if(instance->preset_index > (uint32_t)preset_count - 1) {
        instance->preset_index = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;
    }
}

bool subghz_last_settings_save(SubGhzLastSettings* instance) {
    furi_assert(instance);

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
        if(!flipper_format_write_uint32(
               file, SUBGHZ_LAST_SETTING_FIELD_FREQUENCY, &instance->frequency, 1)) {
            break;
        }
        if(!flipper_format_write_uint32(
               file, SUBGHZ_LAST_SETTING_FIELD_PRESET, &instance->preset_index, 1)) {
            break;
        }
        if(!flipper_format_write_uint32(
               file,
               SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_FEEDBACK_LEVEL,
               &instance->frequency_analyzer_feedback_level,
               1)) {
            break;
        }
        if(!flipper_format_write_float(
               file,
               SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_TRIGGER,
               &instance->frequency_analyzer_trigger,
               1)) {
            break;
        }
        if(!flipper_format_write_bool(
               file,
               SUBGHZ_LAST_SETTING_FIELD_PROTOCOL_FILE_NAMES,
               &instance->protocol_file_names,
               1)) {
            break;
        }
        if(!flipper_format_write_bool(
               file, SUBGHZ_LAST_SETTING_FIELD_HOPPING_ENABLE, &instance->enable_hopping, 1)) {
            break;
        }
        if(!flipper_format_write_uint32(
               file, SUBGHZ_LAST_SETTING_FIELD_IGNORE_FILTER, &instance->ignore_filter, 1)) {
            break;
        }
        if(!flipper_format_write_uint32(
               file, SUBGHZ_LAST_SETTING_FIELD_FILTER, &instance->filter, 1)) {
            break;
        }
        if(!flipper_format_write_float(
               file, SUBGHZ_LAST_SETTING_FIELD_RSSI_THRESHOLD, &instance->rssi, 1)) {
            break;
        }
        if(!flipper_format_write_bool(
               file, SUBGHZ_LAST_SETTING_FIELD_DELETE_OLD, &instance->delete_old_signals, 1)) {
            break;
        }
        if(!flipper_format_write_float(
               file,
               SUBGHZ_LAST_SETTING_FIELD_HOPPING_THRESHOLD,
               &instance->hopping_threshold,
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
