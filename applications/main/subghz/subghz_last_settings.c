#include "subghz_last_settings.h"
#include "subghz_i.h"
#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
#include <lib/subghz/protocols/raw.h>
#endif

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

#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
#define SUBGHZ_LAST_SETTING_DEFAULT_READ_RAW 0
#define SUBGHZ_LAST_SETTING_FIELD_DETECT_RAW "DetectRaw"
#endif

#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY "Frequency"
//#define SUBGHZ_LAST_SETTING_FIELD_PRESET "Preset"
#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_FEEDBACK_LEVEL "FeedbackLevel"
#define SUBGHZ_LAST_SETTING_FIELD_FREQUENCY_ANALYZER_TRIGGER "FATrigger"

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
    //int32_t temp_preset = 0;
    bool frequency_analyzer_feedback_level_was_read = false;
    bool frequency_analyzer_trigger_was_read = false;
#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
    uint32_t temp_read_raw = 0;
#endif

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
#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
        flipper_format_read_uint32(
            fff_data_file, SUBGHZ_LAST_SETTING_FIELD_DETECT_RAW, (uint32_t*)&temp_read_raw, 1);
#endif
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
#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
        instance->detect_raw = SUBGHZ_LAST_SETTING_DEFAULT_READ_RAW;
#endif
    } else {
        instance->frequency = temp_frequency;
        instance->frequency_analyzer_feedback_level =
            frequency_analyzer_feedback_level_was_read ?
                temp_frequency_analyzer_feedback_level :
                SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_FEEDBACK_LEVEL;

        instance->frequency_analyzer_trigger = frequency_analyzer_trigger_was_read ?
                                                   temp_frequency_analyzer_trigger :
                                                   SUBGHZ_LAST_SETTING_FREQUENCY_ANALYZER_TRIGGER;
#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
        instance->detect_raw = temp_read_raw;
#endif

        /*if(temp_preset > (int32_t)preset_count - 1 || temp_preset < 0) {
            FURI_LOG_W(TAG, "Last used preset no found");*/
        instance->preset = SUBGHZ_LAST_SETTING_DEFAULT_PRESET;
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
#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
        if(!flipper_format_insert_or_update_uint32(
               file, SUBGHZ_LAST_SETTING_FIELD_DETECT_RAW, &instance->detect_raw, 1)) {
            break;
        }
#endif
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

#ifdef SUBGHZ_SAVE_DETECT_RAW_SETTING
void subghz_last_settings_set_detect_raw_values(void* context) {
    furi_assert(context);
    SubGhz* instance = (SubGhz*)context;
    bool is_detect_raw = instance->last_settings->detect_raw > 0;
    subghz_receiver_set_filter(
        instance->txrx->receiver, is_detect_raw ? DETECT_RAW_TRUE : DETECT_RAW_FALSE);
    subghz_protocol_decoder_raw_set_auto_mode(
        subghz_receiver_search_decoder_base_by_name(
            instance->txrx->receiver, SUBGHZ_PROTOCOL_RAW_NAME),
        is_detect_raw);
}
#endif