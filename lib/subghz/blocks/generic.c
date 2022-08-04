#include "generic.h"
#include <lib/toolbox/stream/stream.h>
#include <lib/flipper_format/flipper_format_i.h>

#define TAG "SubGhzBlockGeneric"

void subghz_block_generic_get_preset_name(const char* preset_name, string_t preset_str) {
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
    string_set(preset_str, preset_name_temp);
}

bool subghz_block_generic_serialize(
    SubGhzBlockGeneric* instance,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(instance);
    bool res = false;
    string_t temp_str;
    string_init(temp_str);
    do {
        stream_clean(flipper_format_get_raw_stream(flipper_format));
        if(!flipper_format_write_header_cstr(
               flipper_format, SUBGHZ_KEY_FILE_TYPE, SUBGHZ_KEY_FILE_VERSION)) {
            FURI_LOG_E(TAG, "Unable to add header");
            break;
        }

        if(!flipper_format_write_uint32(flipper_format, "Frequency", &preset->frequency, 1)) {
            FURI_LOG_E(TAG, "Unable to add Frequency");
            break;
        }

        subghz_block_generic_get_preset_name(string_get_cstr(preset->name), temp_str);
        if(!flipper_format_write_string_cstr(flipper_format, "Preset", string_get_cstr(temp_str))) {
            FURI_LOG_E(TAG, "Unable to add Preset");
            break;
        }
        if(!strcmp(string_get_cstr(temp_str), "FuriHalSubGhzPresetCustom")) {
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
        uint32_t temp = instance->data_count_bit;
        if(!flipper_format_write_uint32(flipper_format, "Bit", &temp, 1)) {
            FURI_LOG_E(TAG, "Unable to add Bit");
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (instance->data >> i * 8) & 0xFF;
        }

        if(!flipper_format_write_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to add Key");
            break;
        }
        res = true;
    } while(false);
    string_clear(temp_str);
    return res;
}

bool subghz_block_generic_deserialize(SubGhzBlockGeneric* instance, FlipperFormat* flipper_format) {
    furi_assert(instance);
    bool res = false;
    string_t temp_str;
    string_init(temp_str);
    uint32_t temp_data = 0;

    do {
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        if(!flipper_format_read_uint32(flipper_format, "Bit", (uint32_t*)&temp_data, 1)) {
            FURI_LOG_E(TAG, "Missing Bit");
            break;
        }
        instance->data_count_bit = (uint8_t)temp_data;

        uint8_t key_data[sizeof(uint64_t)] = {0};
        if(!flipper_format_read_hex(flipper_format, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Missing Key");
            break;
        }
        for(uint8_t i = 0; i < sizeof(uint64_t); i++) {
            instance->data = instance->data << 8 | key_data[i];
        }

        res = true;
    } while(0);

    string_clear(temp_str);

    return res;
}
