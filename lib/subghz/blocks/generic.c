#include "generic.h"
#include "../types.h"
#include <lib/toolbox/stream/stream.h>
#include <lib/flipper_format/flipper_format_i.h>

#define TAG "SubGhzBlockGeneric"

bool subghz_block_generic_get_preset_name(FuriHalSubGhzPreset preset, string_t preset_str) {
    const char* preset_name;
    switch(preset) {
    case FuriHalSubGhzPresetOok270Async:
        preset_name = "FuriHalSubGhzPresetOok270Async";
        break;
    case FuriHalSubGhzPresetOok650Async:
        preset_name = "FuriHalSubGhzPresetOok650Async";
        break;
    case FuriHalSubGhzPreset2FSKDev238Async:
        preset_name = "FuriHalSubGhzPreset2FSKDev238Async";
        break;
    case FuriHalSubGhzPreset2FSKDev476Async:
        preset_name = "FuriHalSubGhzPreset2FSKDev476Async";
        break;
    default:
        FURI_LOG_E(TAG, "Unknown preset");
        return false;
        break;
    }
    string_set(preset_str, preset_name);
    return true;
}

bool subghz_block_generic_serialize(
    SubGhzBlockGeneric* instance,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset) {
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

        if(!flipper_format_write_uint32(flipper_format, "Frequency", &frequency, 1)) {
            FURI_LOG_E(TAG, "Unable to add Frequency");
            break;
        }
        if(!subghz_block_generic_get_preset_name(preset, temp_str)) {
            break;
        }
        if(!flipper_format_write_string_cstr(flipper_format, "Preset", string_get_cstr(temp_str))) {
            FURI_LOG_E(TAG, "Unable to add Preset");
            break;
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
