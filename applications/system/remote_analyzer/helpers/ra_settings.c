#include "ra_settings.h"

#include <furi.h>
#include <storage/storage.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/subghz/types.h>

#define TAG "RaSettings"

#define RA_SETTINGS_PATH         APP_DATA_PATH("settings")
#define RA_SETTINGS_FILE_TYPE    "Flipper Remote Analyzer Setting File"
#define RA_SETTINGS_FILE_VERSION 1

#define RA_SETTINGS_FIELD_FREQUENCY   "Frequency"
#define RA_SETTINGS_FIELD_PRESET      "Preset"
#define RA_SETTINGS_FIELD_SOUND       "Sound"
#define RA_SETTINGS_FIELD_SWEEP       "Sweep"
#define RA_SETTINGS_FIELD_HOPPING     "Hopping"
#define RA_SETTINGS_FIELD_RSSI        "RSSITrigger"
#define RA_SETTINGS_FIELD_ALERT       "Alert"
#define RA_SETTINGS_FIELD_FA_TRIGGER  "FATrigger"
#define RA_SETTINGS_FIELD_FA_FEEDBACK "FAFeedback"

void ra_settings_load(RaSettings* instance, size_t preset_count) {
    furi_assert(instance);

    instance->frequency = RA_SETTINGS_DEFAULT_FREQUENCY;
    instance->preset_index = RA_SETTINGS_DEFAULT_PRESET;
    instance->sound = false;
    instance->sweep = false;
    instance->hopping = false;
    instance->rssi_trigger = RA_SETTINGS_DEFAULT_RSSI;
    instance->alert = true;
    instance->fa_trigger = RA_SETTINGS_DEFAULT_TRIGGER;
    instance->fa_feedback = 0;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    FuriString* temp_str = furi_string_alloc();
    uint32_t version = 0;

    if(flipper_format_file_open_existing(file, RA_SETTINGS_PATH)) {
        do {
            if(!flipper_format_read_header(file, temp_str, &version)) break;
            if(strcmp(furi_string_get_cstr(temp_str), RA_SETTINGS_FILE_TYPE) != 0 ||
               version != RA_SETTINGS_FILE_VERSION) {
                FURI_LOG_W(TAG, "Settings file type or version mismatch, using defaults");
                break;
            }
            /* Every field is optional: a missing one keeps the default set above. A read
             * searches forward from where the last one stopped, so rewind before each -
             * otherwise one absent field would swallow all the ones after it. */
            flipper_format_rewind(file);
            flipper_format_read_uint32(file, RA_SETTINGS_FIELD_FREQUENCY, &instance->frequency, 1);
            flipper_format_rewind(file);
            flipper_format_read_uint32(file, RA_SETTINGS_FIELD_PRESET, &instance->preset_index, 1);
            flipper_format_rewind(file);
            flipper_format_read_bool(file, RA_SETTINGS_FIELD_SOUND, &instance->sound, 1);
            flipper_format_rewind(file);
            flipper_format_read_bool(file, RA_SETTINGS_FIELD_SWEEP, &instance->sweep, 1);
            flipper_format_rewind(file);
            flipper_format_read_bool(file, RA_SETTINGS_FIELD_HOPPING, &instance->hopping, 1);
            flipper_format_rewind(file);
            flipper_format_read_float(file, RA_SETTINGS_FIELD_RSSI, &instance->rssi_trigger, 1);
            flipper_format_rewind(file);
            flipper_format_read_bool(file, RA_SETTINGS_FIELD_ALERT, &instance->alert, 1);
            flipper_format_rewind(file);
            flipper_format_read_float(
                file, RA_SETTINGS_FIELD_FA_TRIGGER, &instance->fa_trigger, 1);
            flipper_format_rewind(file);
            flipper_format_read_uint32(
                file, RA_SETTINGS_FIELD_FA_FEEDBACK, &instance->fa_feedback, 1);
        } while(false);
    }

    flipper_format_free(file);
    furi_string_free(temp_str);
    furi_record_close(RECORD_STORAGE);

    // A config file that lost presets since the last run must not leave the index past the end
    if(preset_count > 0 && instance->preset_index >= preset_count) {
        instance->preset_index = 0;
    }
    if(!furi_hal_subghz_is_frequency_valid(instance->frequency)) {
        instance->frequency = RA_SETTINGS_DEFAULT_FREQUENCY;
    }
}

bool ra_settings_save(const RaSettings* instance) {
    furi_assert(instance);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool saved = false;

    do {
        if(!flipper_format_file_open_always(file, RA_SETTINGS_PATH)) break;
        if(!flipper_format_write_header_cstr(file, RA_SETTINGS_FILE_TYPE, RA_SETTINGS_FILE_VERSION))
            break;
        if(!flipper_format_write_uint32(file, RA_SETTINGS_FIELD_FREQUENCY, &instance->frequency, 1))
            break;
        if(!flipper_format_write_uint32(file, RA_SETTINGS_FIELD_PRESET, &instance->preset_index, 1))
            break;
        if(!flipper_format_write_bool(file, RA_SETTINGS_FIELD_SOUND, &instance->sound, 1)) break;
        if(!flipper_format_write_bool(file, RA_SETTINGS_FIELD_SWEEP, &instance->sweep, 1)) break;
        if(!flipper_format_write_bool(file, RA_SETTINGS_FIELD_HOPPING, &instance->hopping, 1))
            break;
        if(!flipper_format_write_float(file, RA_SETTINGS_FIELD_RSSI, &instance->rssi_trigger, 1))
            break;
        if(!flipper_format_write_bool(file, RA_SETTINGS_FIELD_ALERT, &instance->alert, 1)) break;
        if(!flipper_format_write_float(
               file, RA_SETTINGS_FIELD_FA_TRIGGER, &instance->fa_trigger, 1))
            break;
        if(!flipper_format_write_uint32(
               file, RA_SETTINGS_FIELD_FA_FEEDBACK, &instance->fa_feedback, 1))
            break;
        saved = true;
    } while(false);

    if(!saved) FURI_LOG_E(TAG, "Failed to save settings");

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
    return saved;
}
