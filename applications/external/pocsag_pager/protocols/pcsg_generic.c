#include "pcsg_generic.h"
#include <lib/toolbox/stream/stream.h>
#include <lib/flipper_format/flipper_format_i.h>
#include "../helpers/pocsag_pager_types.h"

#define TAG "PCSGBlockGeneric"

void pcsg_block_generic_get_preset_name(const char* preset_name, FuriString* preset_str) {
    const char* preset_name_temp;
    if(!strcmp(preset_name, "AM270")) {
        preset_name_temp = "FuriHalSubGhzPresetOok270Async";
    } else if(!strcmp(preset_name, "AM650")) {
        preset_name_temp = "FuriHalSubGhzPresetOok650Async";
    } else if(!strcmp(preset_name, "FM238")) {
        preset_name_temp = "FuriHalSubGhzPreset2FSKDev238Async";
    } else if(!strcmp(preset_name, "FM476")) {
        preset_name_temp = "FuriHalSubGhzPreset2FSKDev476Async";
    } else {
        preset_name_temp = "FuriHalSubGhzPresetCustom";
    }
    furi_string_set(preset_str, preset_name_temp);
}

SubGhzProtocolStatus pcsg_block_generic_serialize(
    PCSGBlockGeneric* instance,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset) {
    furi_assert(instance);
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    FuriString* temp_str;
    temp_str = furi_string_alloc();
    do {
        stream_clean(flipper_format_get_raw_stream(flipper_format));
        if(!flipper_format_write_header_cstr(
               flipper_format, PCSG_KEY_FILE_TYPE, PCSG_KEY_FILE_VERSION)) {
            FURI_LOG_E(TAG, "Unable to add header");
            break;
        }

        if(!flipper_format_write_uint32(flipper_format, "Frequency", &preset->frequency, 1)) {
            FURI_LOG_E(TAG, "Unable to add Frequency");
            break;
        }

        pcsg_block_generic_get_preset_name(furi_string_get_cstr(preset->name), temp_str);
        if(!flipper_format_write_string_cstr(
               flipper_format, "Preset", furi_string_get_cstr(temp_str))) {
            FURI_LOG_E(TAG, "Unable to add Preset");
            break;
        }
        if(!strcmp(furi_string_get_cstr(temp_str), "FuriHalSubGhzPresetCustom")) {
            if(!flipper_format_write_string_cstr(
                   flipper_format, "Custom_preset_module", "CC1101")) {
                FURI_LOG_E(TAG, "Unable to add Custom_preset_module");
                break;
            }
            if(!flipper_format_write_hex(
                   flipper_format, "Custom_preset_data", preset->data, preset->data_size)) {
                FURI_LOG_E(TAG, "Unable to add Custom_preset_data");
                break;
            }
        }
        if(!flipper_format_write_string_cstr(flipper_format, "Protocol", instance->protocol_name)) {
            FURI_LOG_E(TAG, "Unable to add Protocol");
            break;
        }

        if(!flipper_format_write_string(flipper_format, "Ric", instance->result_ric)) {
            FURI_LOG_E(TAG, "Unable to add Ric");
            break;
        }

        if(!flipper_format_write_string(flipper_format, "Message", instance->result_msg)) {
            FURI_LOG_E(TAG, "Unable to add Message");
            break;
        }

        res = SubGhzProtocolStatusOk;
    } while(false);
    furi_string_free(temp_str);
    return res;
}

SubGhzProtocolStatus
    pcsg_block_generic_deserialize(PCSGBlockGeneric* instance, FlipperFormat* flipper_format) {
    furi_assert(instance);
    SubGhzProtocolStatus res = SubGhzProtocolStatusError;
    FuriString* temp_data = furi_string_alloc();
    FuriString* temp_data2 = furi_string_alloc();

    do {
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        if(!flipper_format_read_string(flipper_format, "Ric", temp_data2)) {
            FURI_LOG_E(TAG, "Missing Ric");
            break;
        }
        if(instance->result_ric != NULL) {
            furi_string_set(instance->result_ric, temp_data2);
        } else {
            instance->result_ric = furi_string_alloc_set(temp_data2);
        }

        if(!flipper_format_read_string(flipper_format, "Message", temp_data)) {
            FURI_LOG_E(TAG, "Missing Message");
            break;
        }
        if(instance->result_msg != NULL) {
            furi_string_set(instance->result_msg, temp_data);
        } else {
            instance->result_msg = furi_string_alloc_set(temp_data);
        }

        res = SubGhzProtocolStatusOk;
    } while(0);

    furi_string_free(temp_data);
    furi_string_free(temp_data2);

    return res;
}
