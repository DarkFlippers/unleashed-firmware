#include "subbrute_device.h"

#include <stdint.h>
#include <storage/storage.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/flipper_format/flipper_format.h>
#include <lib/flipper_format/flipper_format_i.h>

#define TAG "SubBruteDevice"

SubBruteDevice* subbrute_device_alloc() {
    SubBruteDevice* instance = malloc(sizeof(SubBruteDevice));

    instance->key_index = 0;

    instance->protocol_info = NULL;
    instance->file_protocol_info = NULL;
    instance->decoder_result = NULL;
    instance->receiver = NULL;
    instance->environment = subghz_environment_alloc();

#ifdef FURI_DEBUG
    subbrute_device_attack_set_default_values(instance, SubBruteAttackCAME12bit433);
#else
    subbrute_device_attack_set_default_values(instance, SubBruteAttackCAME12bit433);
#endif
    return instance;
}

void subbrute_device_free(SubBruteDevice* instance) {
    furi_assert(instance);

    // I don't know how to free this
    instance->decoder_result = NULL;

    if(instance->receiver != NULL) {
        subghz_receiver_free(instance->receiver);
        instance->receiver = NULL;
    }

    subghz_environment_free(instance->environment);
    instance->environment = NULL;

    subbrute_device_free_protocol_info(instance);

    free(instance);
}

uint64_t subbrute_device_add_step(SubBruteDevice* instance, int8_t step) {
    if(step > 0) {
        if((instance->key_index + step) - instance->max_value == 1) {
            instance->key_index = 0x00;
        } else {
            uint64_t value = instance->key_index + step;
            if(value == instance->max_value) {
                instance->key_index = value;
            } else {
                instance->key_index = value % instance->max_value;
            }
        }
    } else {
        if(instance->key_index + step == 0) {
            instance->key_index = 0x00;
        } else if(instance->key_index == 0) {
            instance->key_index = instance->max_value;
        } else {
            uint64_t value = ((instance->key_index + step) + instance->max_value);
            if(value == instance->max_value) {
                instance->key_index = value;
            } else {
                instance->key_index = value % instance->max_value;
            }
        }
    }

    return instance->key_index;
}

bool subbrute_device_save_file(SubBruteDevice* instance, const char* dev_file_name) {
    furi_assert(instance);

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_device_save_file: %s", dev_file_name);
#endif

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    bool result = false;
    do {
        if(!flipper_format_file_open_always(file, dev_file_name)) {
            FURI_LOG_E(TAG, "Failed to open file: %s", dev_file_name);
            break;
        }
        Stream* stream = flipper_format_get_raw_stream(file);
        if(instance->attack == SubBruteAttackLoadFile) {
            subbrute_protocol_file_generate_file(
                stream,
                instance->file_protocol_info->frequency,
                instance->file_protocol_info->preset,
                instance->file_protocol_info->file,
                instance->key_index,
                instance->file_protocol_info->bits,
                instance->file_protocol_info->te,
                instance->file_protocol_info->repeat,
                instance->load_index,
                instance->file_key);
        } else {
            subbrute_protocol_default_generate_file(
                stream,
                instance->protocol_info->frequency,
                instance->protocol_info->preset,
                instance->protocol_info->file,
                instance->key_index,
                instance->protocol_info->bits,
                instance->protocol_info->te,
                instance->protocol_info->repeat);
        }

        result = true;
    } while(false);

    if(!result) {
        FURI_LOG_E(TAG, "subbrute_device_save_file failed!");
    }

    flipper_format_file_close(file);
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

SubBruteFileResult subbrute_device_attack_set(SubBruteDevice* instance, SubBruteAttacks type) {
    furi_assert(instance);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_device_attack_set: %d", type);
#endif
    subbrute_device_attack_set_default_values(instance, type);

    if(type != SubBruteAttackLoadFile) {
        subbrute_device_free_protocol_info(instance);
        instance->protocol_info = subbrute_protocol(type);
    }

    // For non-file types we didn't set SubGhzProtocolDecoderBase
    instance->receiver = subghz_receiver_alloc_init(instance->environment);
    subghz_receiver_set_filter(instance->receiver, SubGhzProtocolFlag_Decodable);
    furi_hal_subghz_reset();

    uint8_t protocol_check_result = SubBruteFileResultProtocolNotFound;
#ifdef FURI_DEBUG
    uint8_t bits;
    uint8_t te;
    uint8_t repeat;
    FuriHalSubGhzPreset preset;
    SubBruteFileProtocol file;
#endif
    if(type != SubBruteAttackLoadFile) {
        instance->decoder_result = subghz_receiver_search_decoder_base_by_name(
            instance->receiver, subbrute_protocol_file(instance->protocol_info->file));

        if(!instance->decoder_result ||
           instance->decoder_result->protocol->type == SubGhzProtocolTypeDynamic) {
            FURI_LOG_E(TAG, "Can't load SubGhzProtocolDecoderBase in phase non-file decoder set");
        } else {
            protocol_check_result = SubBruteFileResultOk;

            // Calc max value
            instance->max_value =
                subbrute_protocol_calc_max_value(instance->attack, instance->protocol_info->bits);
        }
#ifdef FURI_DEBUG
        bits = instance->protocol_info->bits;
        te = instance->protocol_info->te;
        repeat = instance->protocol_info->repeat;
        preset = instance->protocol_info->preset;
        file = instance->protocol_info->file;
#endif
    } else {
        // And here we need to set preset enum
        protocol_check_result = SubBruteFileResultOk;

        // Calc max value
        instance->max_value =
            subbrute_protocol_calc_max_value(instance->attack, instance->file_protocol_info->bits);
#ifdef FURI_DEBUG
        bits = instance->file_protocol_info->bits;
        te = instance->file_protocol_info->te;
        repeat = instance->file_protocol_info->repeat;
        preset = instance->file_protocol_info->preset;
        file = instance->file_protocol_info->file;
#endif
    }

    subghz_receiver_free(instance->receiver);
    instance->receiver = NULL;

    if(protocol_check_result != SubBruteFileResultOk) {
        return SubBruteFileResultProtocolNotFound;
    }

#ifdef FURI_DEBUG
    FURI_LOG_I(
        TAG,
        "subbrute_device_attack_set: %s, bits: %d, preset: %s, file: %s, te: %d, repeat: %d, max_value: %lld",
        subbrute_protocol_name(instance->attack),
        bits,
        subbrute_protocol_preset(preset),
        subbrute_protocol_file(file),
        te,
        repeat,
        instance->max_value);
#endif

    return SubBruteFileResultOk;
}

uint8_t subbrute_device_load_from_file(SubBruteDevice* instance, const char* file_path) {
    furi_assert(instance);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_device_load_from_file: %s", file_path);
#endif
    SubBruteFileResult result = SubBruteFileResultUnknown;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    subbrute_device_free_protocol_info(instance);
    instance->file_protocol_info = malloc(sizeof(SubBruteProtocol));

    FuriString* temp_str;
    temp_str = furi_string_alloc();
    uint32_t temp_data32;

    instance->receiver = subghz_receiver_alloc_init(instance->environment);
    subghz_receiver_set_filter(instance->receiver, SubGhzProtocolFlag_Decodable);
    furi_hal_subghz_reset();

    do {
        if(!flipper_format_file_open_existing(fff_data_file, file_path)) {
            FURI_LOG_E(TAG, "Error open file %s", file_path);
            result = SubBruteFileResultErrorOpenFile;
            break;
        }
        if(!flipper_format_read_header(fff_data_file, temp_str, &temp_data32)) {
            FURI_LOG_E(TAG, "Missing or incorrect header");
            result = SubBruteFileResultMissingOrIncorrectHeader;
            break;
        }

        // Frequency
        if(flipper_format_read_uint32(fff_data_file, "Frequency", &temp_data32, 1)) {
            instance->file_protocol_info->frequency = temp_data32;
            if(!furi_hal_subghz_is_tx_allowed(instance->file_protocol_info->frequency)) {
                result = SubBruteFileResultFrequencyNotAllowed;
                break;
            }
        } else {
            FURI_LOG_E(TAG, "Missing or incorrect Frequency");
            result = SubBruteFileResultMissingOrIncorrectFrequency;
            break;
        }

        // Preset
        if(!flipper_format_read_string(fff_data_file, "Preset", temp_str)) {
            FURI_LOG_E(TAG, "Preset FAIL");
            result = SubBruteFileResultPresetInvalid;
        } else {
            instance->file_protocol_info->preset = subbrute_protocol_convert_preset(temp_str);
        }

        const char* protocol_file = NULL;
        // Protocol
        if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            result = SubBruteFileResultMissingProtocol;
            break;
        } else {
            instance->file_protocol_info->file = subbrute_protocol_file_protocol_name(temp_str);
            protocol_file = subbrute_protocol_file(instance->file_protocol_info->file);
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Protocol: %s", protocol_file);
#endif
        }

        instance->decoder_result =
            subghz_receiver_search_decoder_base_by_name(instance->receiver, protocol_file);

        if(!instance->decoder_result || strcmp(protocol_file, "RAW") == 0) {
            FURI_LOG_E(TAG, "RAW unsupported");
            result = SubBruteFileResultProtocolNotSupported;
            break;
        }

        if(instance->decoder_result->protocol->type == SubGhzProtocolTypeDynamic) {
            FURI_LOG_E(TAG, "Protocol is dynamic - not supported");
            result = SubBruteFileResultDynamicProtocolNotValid;
            break;
        }
#ifdef FURI_DEBUG
        else {
            FURI_LOG_D(TAG, "Decoder: %s", instance->decoder_result->protocol->name);
        }
#endif

        // Bit
        if(!flipper_format_read_uint32(fff_data_file, "Bit", &temp_data32, 1)) {
            FURI_LOG_E(TAG, "Missing or incorrect Bit");
            result = SubBruteFileResultMissingOrIncorrectBit;
            break;
        } else {
            instance->file_protocol_info->bits = temp_data32;
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Bit: %d", instance->file_protocol_info->bits);
#endif
        }

        // Key
        if(!flipper_format_read_string(fff_data_file, "Key", temp_str)) {
            FURI_LOG_E(TAG, "Missing or incorrect Key");
            result = SubBruteFileResultMissingOrIncorrectKey;
            break;
        } else {
            snprintf(
                instance->file_key,
                sizeof(instance->file_key),
                "%s",
                furi_string_get_cstr(temp_str));
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Key: %s", instance->file_key);
#endif
        }

        // TE
        if(!flipper_format_read_uint32(fff_data_file, "TE", &temp_data32, 1)) {
            FURI_LOG_E(TAG, "Missing or incorrect TE");
            //result = SubBruteFileResultMissingOrIncorrectTe;
            //break;
        } else {
            instance->file_protocol_info->te = temp_data32 != 0 ? temp_data32 : 0;
        }

        // Repeat
        if(flipper_format_read_uint32(fff_data_file, "Repeat", &temp_data32, 1)) {
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Repeat: %ld", temp_data32);
#endif
            instance->file_protocol_info->repeat = (uint8_t)temp_data32;
        } else {
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Repeat: 3 (default)");
#endif
            instance->file_protocol_info->repeat = 3;
        }

        result = SubBruteFileResultOk;
    } while(0);

    furi_string_free(temp_str);
    flipper_format_file_close(fff_data_file);
    flipper_format_free(fff_data_file);
    furi_record_close(RECORD_STORAGE);

    subghz_receiver_free(instance->receiver);

    instance->decoder_result = NULL;
    instance->receiver = NULL;

    if(result == SubBruteFileResultOk) {
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "Loaded successfully");
#endif
    } else {
        subbrute_device_free_protocol_info(instance);
    }

    return result;
}

void subbrute_device_attack_set_default_values(
    SubBruteDevice* instance,
    SubBruteAttacks default_attack) {
    furi_assert(instance);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_device_attack_set_default_values");
#endif
    instance->attack = default_attack;
    instance->key_index = 0x00;
    instance->load_index = 0x00;
    memset(instance->current_key, 0, sizeof(instance->current_key));

    if(default_attack != SubBruteAttackLoadFile) {
        memset(instance->file_key, 0, sizeof(instance->file_key));

        instance->max_value = (uint64_t)0x00;
    }
}

const char* subbrute_device_error_get_desc(SubBruteFileResult error_id) {
    const char* result;
    switch(error_id) {
    case(SubBruteFileResultOk):
        result = "OK";
        break;
    case(SubBruteFileResultErrorOpenFile):
        result = "invalid name/path";
        break;
    case(SubBruteFileResultMissingOrIncorrectHeader):
        result = "Missing or incorrect header";
        break;
    case(SubBruteFileResultFrequencyNotAllowed):
        result = "Invalid frequency!";
        break;
    case(SubBruteFileResultMissingOrIncorrectFrequency):
        result = "Missing or incorrect Frequency";
        break;
    case(SubBruteFileResultPresetInvalid):
        result = "Preset FAIL";
        break;
    case(SubBruteFileResultMissingProtocol):
        result = "Missing Protocol";
        break;
    case(SubBruteFileResultProtocolNotSupported):
        result = "RAW unsupported";
        break;
    case(SubBruteFileResultDynamicProtocolNotValid):
        result = "Dynamic protocol unsupported";
        break;
    case(SubBruteFileResultProtocolNotFound):
        result = "Protocol not found";
        break;
    case(SubBruteFileResultMissingOrIncorrectBit):
        result = "Missing or incorrect Bit";
        break;
    case(SubBruteFileResultMissingOrIncorrectKey):
        result = "Missing or incorrect Key";
        break;
    case(SubBruteFileResultMissingOrIncorrectTe):
        result = "Missing or incorrect TE";
        break;
    case SubBruteFileResultUnknown:
    default:
        result = "Unknown error";
        break;
    }
    return result;
}

void subbrute_device_free_protocol_info(SubBruteDevice* instance) {
    furi_assert(instance);
    instance->protocol_info = NULL;
    if(instance->file_protocol_info) {
        free(instance->file_protocol_info);
    }
    instance->file_protocol_info = NULL;
}