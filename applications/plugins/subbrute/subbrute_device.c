#include "subbrute_device.h"
#include "subbrute_i.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_subghz.h>

#include <stdint.h>
#include <stdbool.h>

#include <lib/subghz/types.h>
#include <lib/subghz/protocols/base.h>

#include <storage/storage.h>
#include <dialogs/dialogs.h>
#include <stream/stream.h>
#include <stream/buffered_file_stream.h>
#include <lib/toolbox/path.h>
#include <lib/flipper_format/flipper_format_i.h>

#define TAG "SubBruteDevice"

/**
 * List of protocols
 */
static const char* protocol_came = "CAME";
static const char* protocol_cham_code = "Cham_Code";
static const char* protocol_linear = "Linear";
static const char* protocol_nice_flo = "Nice FLO";
static const char* protocol_princeton = "Princeton";
static const char* protocol_raw = "RAW";

/**
 * Values to not use less memory for packet parse operations
 */
static const char* subbrute_key_file_start =
    "Filetype: Flipper SubGhz Key File\nVersion: 1\nFrequency: %u\nPreset: %s\nProtocol: %s\nBit: %d";
static const char* subbrute_key_file_key = "%s\nKey: %s\n";
static const char* subbrute_key_file_princeton_end = "%s\nKey: %s\nTE: %d\n";
static const char* subbrute_key_small_no_tail = "Bit: %d\nKey: %s\n";
static const char* subbrute_key_small_with_tail = "Bit: %d\nKey: %s\nTE: %d\n";

// Why nobody set in as const in all codebase?
static const char* preset_ook270_async = "FuriHalSubGhzPresetOok270Async";
static const char* preset_ook650_async = "FuriHalSubGhzPresetOok650Async";
static const char* preset_2fsk_dev238_async = "FuriHalSubGhzPreset2FSKDev238Async";
static const char* preset_2fsk_dev476_async = "FuriHalSubGhzPreset2FSKDev476Async";
static const char* preset_msk99_97_kb_async = "FuriHalSubGhzPresetMSK99_97KbAsync";
static const char* preset_gfs99_97_kb_async = "FuriHalSubGhzPresetGFS99_97KbAsync";

SubBruteDevice* subbrute_device_alloc() {
    SubBruteDevice* instance = malloc(sizeof(SubBruteDevice));

    instance->state = SubBruteDeviceStateIDLE;
    instance->key_index = 0;

    string_init(instance->load_path);
    string_init(instance->preset_name);
    string_init(instance->protocol_name);

    instance->decoder_result = NULL;
    instance->receiver = NULL;
    instance->environment = subghz_environment_alloc();

    subbrute_device_attack_set_default_values(instance, SubBruteAttackCAME12bit307);

    return instance;
}

void subbrute_device_free(SubBruteDevice* instance) {
    furi_assert(instance);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_device_free");
#endif

    // I don't know how to free this
    instance->decoder_result = NULL;

    if(instance->receiver != NULL) {
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "subghz_receiver_free");
#endif
        subghz_receiver_free(instance->receiver);
        instance->receiver = NULL;
    }

    subghz_environment_free(instance->environment);
    instance->environment = NULL;

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "before free");
#endif

    string_clear(instance->load_path);
    string_clear(instance->preset_name);
    string_clear(instance->protocol_name);

    free(instance);
}

bool subbrute_device_save_file(SubBruteDevice* instance, const char* dev_file_name) {
    furi_assert(instance);

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_device_save_file: %s", dev_file_name);
#endif
    bool result = subbrute_device_create_packet_parsed(instance, instance->key_index, false);

    if(!result) {
        FURI_LOG_E(TAG, "subbrute_device_create_packet_parsed failed!");
        //subbrute_device_notification_message(instance, &sequence_error);
        return false;
    }

    Storage* storage = furi_record_open(RECORD_STORAGE);
    Stream* stream = buffered_file_stream_alloc(storage);

    result = false;
    do {
        if(!buffered_file_stream_open(stream, dev_file_name, FSAM_READ_WRITE, FSOM_OPEN_ALWAYS)) {
            buffered_file_stream_close(stream);
            break;
        }
        stream_write_cstring(stream, instance->payload);

        result = true;
    } while(false);

    buffered_file_stream_close(stream);
    stream_free(stream);
    if(!result) {
        FURI_LOG_E(TAG, "stream_write_string failed!");
        //subbrute_device_notification_message(instance, &sequence_error);
    }

    furi_record_close(RECORD_STORAGE);

    return result;
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

bool subbrute_device_create_packet_parsed(SubBruteDevice* instance, uint64_t step, bool small) {
    furi_assert(instance);

    //char step_payload[32];
    //memset(step_payload, '0', sizeof(step_payload));
    memset(instance->payload, 0, sizeof(instance->payload));
    string_t candidate;
    string_init(candidate);

    if(instance->attack == SubBruteAttackLoadFile) {
        if(step >= sizeof(instance->file_key)) {
            return false;
        }
        char subbrute_payload_byte[4];
        string_set_str(candidate, instance->file_key);
        snprintf(subbrute_payload_byte, 4, "%02X ", (uint8_t)step);
        string_replace_at(candidate, instance->load_index * 3, 3, subbrute_payload_byte);
        //snprintf(step_payload, sizeof(step_payload), "%02X", (uint8_t)instance->file_key[step]);
    } else {
        //snprintf(step_payload, sizeof(step_payload), "%16X", step);
        //snprintf(step_payload, sizeof(step_payload), "%016llX", step);
        string_t buffer;
        string_init(buffer);
        string_init_printf(buffer, "%16X", step);
        int j = 0;
        string_set_str(candidate, "                       ");
        for(uint8_t i = 0; i < 16; i++) {
            if(string_get_char(buffer, i) != ' ') {
                string_set_char(candidate, i + j, string_get_char(buffer, i));
            } else {
                string_set_char(candidate, i + j, '0');
            }
            if(i % 2 != 0) {
                j++;
            }
        }
        string_clear(buffer);
    }

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "candidate: %s, step: %d", string_get_cstr(candidate), step);
#endif

    if(small) {
        if(instance->has_tail) {
            snprintf(
                instance->payload,
                sizeof(instance->payload),
                subbrute_key_small_with_tail,
                instance->bit,
                string_get_cstr(candidate),
                instance->te);
        } else {
            snprintf(
                instance->payload,
                sizeof(instance->payload),
                subbrute_key_small_no_tail,
                instance->bit,
                string_get_cstr(candidate));
        }
    } else {
        if(instance->has_tail) {
            snprintf(
                instance->payload,
                sizeof(instance->payload),
                subbrute_key_file_princeton_end,
                instance->file_template,
                string_get_cstr(candidate),
                instance->te);
        } else {
            snprintf(
                instance->payload,
                sizeof(instance->payload),
                subbrute_key_file_key,
                instance->file_template,
                string_get_cstr(candidate));
        }
    }

#ifdef FURI_DEBUG
    //FURI_LOG_D(TAG, "payload: %s", instance->payload);
#endif

    string_clear(candidate);

    return true;
}

SubBruteFileResult subbrute_device_attack_set(SubBruteDevice* instance, SubBruteAttacks type) {
    furi_assert(instance);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_device_attack_set: %d", type);
#endif
    subbrute_device_attack_set_default_values(instance, type);
    switch(type) {
    case SubBruteAttackLoadFile:
        // In this case values must be already set
        //        file_result =
        //            subbrute_device_load_from_file(instance, string_get_cstr(instance->load_path));
        //        if(file_result != SubBruteFileResultOk) {
        //            // Failed load file so failed to set attack type
        //            return file_result; // RETURN
        //        }
        break;
    case SubBruteAttackCAME12bit307:
    case SubBruteAttackCAME12bit433:
    case SubBruteAttackCAME12bit868:
        if(type == SubBruteAttackCAME12bit307) {
            instance->frequency = 307800000;
        } else if(type == SubBruteAttackCAME12bit433) {
            instance->frequency = 433920000;
        } else /* ALWAYS TRUE if(type == SubBruteAttackCAME12bit868) */ {
            instance->frequency = 868350000;
        }
        instance->bit = 12;
        string_set_str(instance->protocol_name, protocol_came);
        string_set_str(instance->preset_name, preset_ook650_async);
        break;
    case SubBruteAttackChamberlain9bit300:
        instance->frequency = 300000000;
        instance->bit = 9;
        string_set_str(instance->protocol_name, protocol_cham_code);
        string_set_str(instance->preset_name, preset_ook650_async);
        break;
    case SubBruteAttackChamberlain9bit315:
        instance->frequency = 315000000;
        instance->bit = 9;
        string_set_str(instance->protocol_name, protocol_cham_code);
        string_set_str(instance->preset_name, preset_ook650_async);
        break;
    case SubBruteAttackChamberlain9bit390:
        instance->frequency = 390000000;
        instance->bit = 9;
        string_set_str(instance->protocol_name, protocol_cham_code);
        string_set_str(instance->preset_name, preset_ook650_async);
        break;
    case SubBruteAttackLinear10bit300:
        instance->frequency = 300000000;
        instance->bit = 10;
        string_set_str(instance->protocol_name, protocol_linear);
        string_set_str(instance->preset_name, preset_ook650_async);
        break;
    case SubBruteAttackLinear10bit310:
        instance->frequency = 310000000;
        instance->bit = 10;
        string_set_str(instance->protocol_name, protocol_linear);
        string_set_str(instance->preset_name, preset_ook650_async);
        break;
    case SubBruteAttackNICE12bit433:
        instance->frequency = 433920000;
        instance->bit = 12;
        string_set_str(instance->protocol_name, protocol_nice_flo);
        string_set_str(instance->preset_name, preset_ook650_async);
        break;
    case SubBruteAttackNICE12bit868:
        instance->frequency = 868350000;
        instance->bit = 12;
        string_set_str(instance->protocol_name, protocol_nice_flo);
        string_set_str(instance->preset_name, preset_ook650_async);
        break;
    default:
        FURI_LOG_E(TAG, "Unknown attack type: %d", type);
        return SubBruteFileResultProtocolNotFound; // RETURN
    }

    if(!furi_hal_subghz_is_tx_allowed(instance->frequency)) {
        FURI_LOG_E(TAG, "Frequency invalid: %d", instance->frequency);
        return SubBruteFileResultMissingOrIncorrectFrequency; // RETURN
    }

    // For non-file types we didn't set SubGhzProtocolDecoderBase
    instance->receiver = subghz_receiver_alloc_init(instance->environment);
    subghz_receiver_set_filter(instance->receiver, SubGhzProtocolFlag_Decodable);
    furi_hal_subghz_reset();

    uint8_t protocol_check_result = SubBruteFileResultProtocolNotFound;
    if(type != SubBruteAttackLoadFile) {
        instance->decoder_result = subghz_receiver_search_decoder_base_by_name(
            instance->receiver, string_get_cstr(instance->protocol_name));

        if(!instance->decoder_result ||
           instance->decoder_result->protocol->type == SubGhzProtocolTypeDynamic) {
            FURI_LOG_E(TAG, "Can't load SubGhzProtocolDecoderBase in phase non-file decoder set");
        } else {
            protocol_check_result = SubBruteFileResultOk;
        }
    } else {
        // And here we need to set preset enum
        instance->preset = subbrute_device_convert_preset(string_get_cstr(instance->preset_name));
        protocol_check_result = SubBruteFileResultOk;
    }

    subghz_receiver_free(instance->receiver);
    instance->receiver = NULL;

    if(protocol_check_result != SubBruteFileResultOk) {
        return SubBruteFileResultProtocolNotFound;
    }

    instance->has_tail =
        (strcmp(string_get_cstr(instance->protocol_name), protocol_princeton) == 0);

    // Calc max value
    if(instance->attack == SubBruteAttackLoadFile) {
        instance->max_value = 0xFF;
    } else {
        string_t max_value_s;
        string_init(max_value_s);
        for(uint8_t i = 0; i < instance->bit; i++) {
            string_cat_printf(max_value_s, "1");
        }
        instance->max_value = (uint64_t)strtol(string_get_cstr(max_value_s), NULL, 2);
        string_clear(max_value_s);
    }

    // Now we are ready to set file template for using in the future with snprintf
    // for sending attack payload
    snprintf(
        instance->file_template,
        sizeof(instance->file_template),
        subbrute_key_file_start,
        instance->frequency,
        string_get_cstr(instance->preset_name),
        string_get_cstr(instance->protocol_name),
        instance->bit);
//    strncat(instance->file_template, "\n", sizeof(instance->file_template));
//    strncat(instance->file_template, subbrute_key_file_key, sizeof(instance->file_template));
//    if(instance->has_tail) {
//        strncat(
//            instance->file_template,
//            subbrute_key_file_princeton_end,
//            sizeof(instance->file_template));
//    }
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "tail: %d, file_template: %s", instance->has_tail, instance->file_template);
#endif

    // Init payload
    subbrute_device_create_packet_parsed(instance, instance->key_index, false);

    return SubBruteFileResultOk;
}

uint8_t subbrute_device_load_from_file(SubBruteDevice* instance, string_t file_path) {
    furi_assert(instance);
#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_device_load_from_file: %s", string_get_cstr(file_path));
#endif
    SubBruteFileResult result = SubBruteFileResultUnknown;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* fff_data_file = flipper_format_file_alloc(storage);

    string_t temp_str;
    string_init(temp_str);
    uint32_t temp_data32;

    instance->receiver = subghz_receiver_alloc_init(instance->environment);
    subghz_receiver_set_filter(instance->receiver, SubGhzProtocolFlag_Decodable);
    furi_hal_subghz_reset();

    do {
        if(!flipper_format_file_open_existing(fff_data_file, string_get_cstr(file_path))) {
            FURI_LOG_E(TAG, "Error open file %s", string_get_cstr(file_path));
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
            instance->frequency = temp_data32;
            if(!furi_hal_subghz_is_tx_allowed(instance->frequency)) {
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
            string_init_set_str(instance->preset_name, string_get_cstr(temp_str));
        }

        // Protocol
        if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            result = SubBruteFileResultMissingProtocol;
            break;
        } else {
            string_init_set_str(instance->protocol_name, string_get_cstr(temp_str));
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Protocol: %s", string_get_cstr(instance->protocol_name));
#endif
        }

        instance->decoder_result = subghz_receiver_search_decoder_base_by_name(
            instance->receiver, string_get_cstr(instance->protocol_name));

        if(!instance->decoder_result ||
           strcmp(string_get_cstr(instance->protocol_name), "RAW") == 0) {
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

        //        instance->decoder_result = subghz_receiver_search_decoder_base_by_name(
        //            instance->receiver, string_get_cstr(instance->protocol_name));
        //
        //        if(!instance->decoder_result) {
        //            FURI_LOG_E(TAG, "Protocol not found");
        //            result = SubBruteFileResultProtocolNotFound;
        //            break;
        //        }

        // Bit
        if(!flipper_format_read_uint32(fff_data_file, "Bit", &temp_data32, 1)) {
            FURI_LOG_E(TAG, "Missing or incorrect Bit");
            result = SubBruteFileResultMissingOrIncorrectBit;
            break;
        } else {
            instance->bit = temp_data32;
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Bit: %d", instance->bit);
#endif
        }

        // Key
        if(!flipper_format_read_string(fff_data_file, "Key", temp_str)) {
            FURI_LOG_E(TAG, "Missing or incorrect Key");
            result = SubBruteFileResultMissingOrIncorrectKey;
            break;
        } else {
            snprintf(
                instance->file_key, sizeof(instance->file_key), "%s", string_get_cstr(temp_str));
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
            instance->te = temp_data32;
            instance->has_tail = true;
        }

        // Repeat
        if(flipper_format_read_uint32(fff_data_file, "Repeat", &temp_data32, 1)) {
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Repeat: %d", temp_data32);
#endif
            instance->repeat = temp_data32;
        } else {
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Repeat: 3 (default)");
#endif
            instance->repeat = 3;
        }

        result = SubBruteFileResultOk;
    } while(0);

    string_clear(temp_str);
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
    memset(instance->file_template, 0, sizeof(instance->file_template));
    memset(instance->current_key, 0, sizeof(instance->current_key));
    memset(instance->text_store, 0, sizeof(instance->text_store));
    memset(instance->payload, 0, sizeof(instance->payload));

    if(default_attack != SubBruteAttackLoadFile) {
        memset(instance->file_key, 0, sizeof(instance->file_key));

        instance->max_value = (uint64_t)0x00;

        string_clear(instance->protocol_name);
        string_clear(instance->preset_name);

        string_clear(instance->load_path);
        string_init(instance->load_path);

        string_init_set_str(instance->protocol_name, protocol_raw);
        string_init_set_str(instance->preset_name, preset_ook650_async);
        instance->preset = FuriHalSubGhzPresetOok650Async;

        instance->repeat = 5;
        instance->te = 0;
        instance->has_tail = false;
    }
#ifdef FURI_DEBUG
    FURI_LOG_D(
        TAG, "subbrute_device_attack_set_default_values done. has_tail: %d", instance->has_tail);
    //furi_delay_ms(250);
#endif
}

FuriHalSubGhzPreset subbrute_device_convert_preset(const char* preset_name) {
    string_t preset;
    string_init_set_str(preset, preset_name);
    FuriHalSubGhzPreset preset_value;
    if(string_cmp_str(preset, preset_ook270_async) == 0) {
        preset_value = FuriHalSubGhzPresetOok270Async;
    } else if(string_cmp_str(preset, preset_ook650_async) == 0) {
        preset_value = FuriHalSubGhzPresetOok650Async;
    } else if(string_cmp_str(preset, preset_2fsk_dev238_async) == 0) {
        preset_value = FuriHalSubGhzPreset2FSKDev238Async;
    } else if(string_cmp_str(preset, preset_2fsk_dev476_async) == 0) {
        preset_value = FuriHalSubGhzPreset2FSKDev476Async;
    } else if(string_cmp_str(preset, preset_msk99_97_kb_async) == 0) {
        preset_value = FuriHalSubGhzPresetMSK99_97KbAsync;
    } else if(string_cmp_str(preset, preset_gfs99_97_kb_async) == 0) {
        preset_value = FuriHalSubGhzPresetMSK99_97KbAsync;
    } else {
        preset_value = FuriHalSubGhzPresetCustom;
    }

    string_clear(preset);
    return preset_value;
}
