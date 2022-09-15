#include "subghz_setting.h"
#include "subghz_i.h"

#include "subghz_last_setting.h"

#include <furi.h>
#include <m-list.h>
#include "furi_hal_subghz.h"
#include "furi_hal_subghz_configs.h"
#include <lib/subghz/protocols/raw.h>

#define TAG "SubGhzLastSetting"

#define SUBGHZ_LAST_SETTING_FILE_TYPE "Flipper SubGhz Last Setting File"
#define SUBGHZ_LAST_SETTING_FILE_VERSION 1
#define SUBGHZ_LAST_SETTING_DEFAULT_PRESET "AM650"
#define SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY 433920000

SubGhzLastSetting* subghz_last_setting_alloc(void) {
    SubGhzLastSetting* instance = malloc(sizeof(SubGhzLastSetting));
    string_init(instance->preset_name);

    return instance;
}

void subghz_last_setting_free(SubGhzLastSetting* instance) {
    furi_assert(instance);

    string_clear(instance->preset_name);
    free(instance);
}

void subghz_last_setting_load(SubGhzLastSetting* instance, const char* file_path) {
    furi_assert(instance);
    string_init(instance->preset_name);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    string_t temp_preset;
    string_init(temp_preset);
    uint32_t temp_frequency = 0; // Default 433920000
    uint32_t temp_hopping = 0; // Default 0
    //uint32_t temp_detect_raw = 0;  Default 2
    int32_t temp_rssi_threshold = 0; // Default -72

    if(FSE_OK == storage_sd_status(storage) && file_path &&
       flipper_format_file_open_existing(fff_data_file, file_path)) {
        flipper_format_read_string(fff_data_file, "Preset", temp_preset);
        flipper_format_read_uint32(fff_data_file, "Frequency", (uint32_t*)&temp_frequency, 1);
        flipper_format_read_uint32(fff_data_file, "Hopping", (uint32_t*)&temp_hopping, 1);
        //flipper_format_read_uint32(fff_data_file, "DetectRaw", (uint32_t*)&temp_detect_raw, 1);
        flipper_format_read_int32(fff_data_file, "Rssi", (int32_t*)&temp_rssi_threshold, 1);
    } else {
        FURI_LOG_E(TAG, "Error open file %s", file_path);
    }

    if(string_empty_p(temp_preset)) {
        //FURI_LOG_I(TAG, "Last used preset not found");
        string_set(instance->preset_name, SUBGHZ_LAST_SETTING_DEFAULT_PRESET);
    } else {
        string_set(instance->preset_name, temp_preset);
    }

    if(temp_frequency == 0 || !furi_hal_subghz_is_tx_allowed(temp_frequency)) {
        //FURI_LOG_I(TAG, "Last used frequency not found or can't be used!");
        instance->frequency = SUBGHZ_LAST_SETTING_DEFAULT_FREQUENCY;
    } else {
        instance->frequency = temp_frequency;
    }

    /*if(temp_detect_raw == 0) {
        instance->detect_raw = SubGhzProtocolFlag_Decodable;
    } else {
        instance->detect_raw = temp_detect_raw;
    }*/

    if(temp_rssi_threshold == 0) {
        instance->rssi_threshold = -72;
    } else {
        instance->rssi_threshold = temp_rssi_threshold;
    }
    instance->hopping = temp_hopping;

    string_clear(temp_preset);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);
}

bool subghz_last_setting_save(SubGhzLastSetting* instance, const char* file_path) {
    furi_assert(instance);

    bool saved = false;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);

    do {
        if(FSE_OK != storage_sd_status(storage)) break;

        // Open file
        if(!flipper_format_file_open_always(file, file_path)) break;

        // Write header
        if(!flipper_format_write_header_cstr(
               file, SUBGHZ_LAST_SETTING_FILE_TYPE, SUBGHZ_LAST_SETTING_FILE_VERSION))
            break;

        //FURI_LOG_D(TAG, "Preset %s", string_get_cstr(instance->preset_name));
        if(!flipper_format_insert_or_update_string_cstr(
               file, "Preset", string_get_cstr(instance->preset_name)))
            break;
        if(!flipper_format_insert_or_update_uint32(file, "Frequency", &instance->frequency, 1))
            break;
        if(!flipper_format_insert_or_update_uint32(file, "Hopping", &instance->hopping, 1)) break;
        //if(!flipper_format_insert_or_update_uint32(file, "DetectRaw", &instance->detect_raw, 1))
        //    break;
        if(!flipper_format_insert_or_update_int32(file, "Rssi", &instance->rssi_threshold, 1))
            break;

        saved = true;
    } while(0);

    if(!saved) {
        FURI_LOG_E(TAG, "Error save file %s", file_path);
    }

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return saved;
}

void subghz_last_setting_set_receiver_values(SubGhzLastSetting* instance, SubGhzReceiver* receiver) {
    /*subghz_receiver_set_filter(receiver, instance->detect_raw);

    subghz_protocol_decoder_raw_set_auto_mode(
        subghz_receiver_search_decoder_base_by_name(receiver, SUBGHZ_PROTOCOL_RAW_NAME),
        (instance->detect_raw != SubGhzProtocolFlag_Decodable));*/

    subghz_protocol_decoder_raw_set_rssi_threshold(
        subghz_receiver_search_decoder_base_by_name(receiver, SUBGHZ_PROTOCOL_RAW_NAME),
        instance->rssi_threshold);
}