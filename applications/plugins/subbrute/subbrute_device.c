#include "subbrute_device.h"

#include <lib/toolbox/stream/stream.h>
#include <stdint.h>
#include <stream/buffered_file_stream.h>
#include <lib/flipper_format/flipper_format_i.h>

#define TAG "SubBruteDevice"

#define SUBBRUTE_TX_TIMEOUT 5
#define SUBBRUTE_MANUAL_TRANSMIT_INTERVAL 400

/**
 * Values to not use less memory for packet parse operations
 */
static const char* subbrute_key_file_start =
    "Filetype: Flipper SubGhz Key File\nVersion: 1\nFrequency: %u\nPreset: %s\nProtocol: %s\nBit: %d";
static const char* subbrute_key_file_key = "%s\nKey: %s\nRepeat: %d\n";
static const char* subbrute_key_file_key_with_tail = "%s\nKey: %s\nTE: %d\nRepeat: %d\n";
static const char* subbrute_key_small_no_tail = "Bit: %d\nKey: %s\nRepeat: %d\nRepeat: %d\n";
static const char* subbrute_key_small_with_tail = "Bit: %d\nKey: %s\nTE: %d\nRepeat: %d\n";

SubBruteDevice* subbrute_device_alloc() {
    SubBruteDevice* instance = malloc(sizeof(SubBruteDevice));

    instance->state = SubBruteDeviceStateIDLE;
    instance->key_index = 0;
    instance->worker_running = false;
    instance->last_time_tx_data = 0;

    instance->thread = furi_thread_alloc();
    furi_thread_set_name(instance->thread, "SubBruteAttackWorker");
    furi_thread_set_stack_size(instance->thread, 2048);
    furi_thread_set_context(instance->thread, instance);
    furi_thread_set_callback(instance->thread, subbrute_worker_thread);

    instance->context = NULL;
    instance->callback = NULL;

    instance->protocol_info = NULL;
    instance->decoder_result = NULL;
    instance->transmitter = NULL;
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

    if(instance->transmitter != NULL) {
        subghz_transmitter_free(instance->transmitter);
        instance->transmitter = NULL;
    }

    subghz_environment_free(instance->environment);
    instance->environment = NULL;

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "before free");
#endif

    furi_thread_free(instance->thread);
    subbrute_device_free_protocol_info(instance);

    free(instance);
}

/**
 * Entrypoint for worker
 *
 * @param context SubBruteWorker*
 * @return 0 if ok
 */
int32_t subbrute_worker_thread(void* context) {
    furi_assert(context);
    SubBruteDevice* instance = (SubBruteDevice*)context;

    if(!instance->worker_running) {
        FURI_LOG_W(TAG, "Worker is not set to running state!");
        return -1;
    }
    if(instance->state != SubBruteDeviceStateReady &&
       instance->state != SubBruteDeviceStateFinished) {
        FURI_LOG_W(TAG, "Invalid state for running worker! State: %d", instance->state);
        return -2;
    }
#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Worker start");
#endif

    SubBruteDeviceState local_state = instance->state = SubBruteDeviceStateTx;
    subbrute_device_send_callback(instance);

    FlipperFormat* flipper_format = flipper_format_string_alloc();

    while(instance->worker_running) {
        if(!subbrute_device_create_packet_parsed(
               instance, flipper_format, instance->key_index, true)) {
            FURI_LOG_W(TAG, "Error creating packet! BREAK");
            instance->worker_running = false;
            local_state = SubBruteDeviceStateIDLE;
            break;
        }
        subbrute_device_subghz_transmit(instance, flipper_format);

        if(instance->key_index + 1 > instance->max_value) {
#ifdef FURI_DEBUG
            FURI_LOG_I(TAG, "Worker finished to end");
#endif
            local_state = SubBruteDeviceStateFinished;
            break;
        }
        instance->key_index++;

        furi_delay_ms(SUBBRUTE_TX_TIMEOUT);
    }

    flipper_format_free(flipper_format);

    instance->worker_running = false; // Because we have error states
    instance->state = local_state == SubBruteDeviceStateTx ? SubBruteDeviceStateReady :
                                                             local_state;
    subbrute_device_send_callback(instance);

#ifdef FURI_DEBUG
    FURI_LOG_I(TAG, "Worker stop");
#endif
    return 0;
}

bool subbrute_worker_start(SubBruteDevice* instance) {
    furi_assert(instance);

    if(instance->worker_running) {
        FURI_LOG_W(TAG, "Worker is already running!");
        return false;
    }
    if(instance->state != SubBruteDeviceStateReady &&
       instance->state != SubBruteDeviceStateFinished) {
        FURI_LOG_W(TAG, "Worker cannot start, invalid device state: %d", instance->state);
        return false;
    }
    if(instance->protocol_info == NULL) {
        FURI_LOG_W(TAG, "Worker cannot start, protocol_info is NULL!");
        return false;
    }

    instance->worker_running = true;
    furi_thread_start(instance->thread);

    return true;
}

void subbrute_worker_stop(SubBruteDevice* instance) {
    furi_assert(instance);

    instance->worker_running = false;

    furi_thread_join(instance->thread);

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();
}

SubBruteAttacks subbrute_device_get_attack(SubBruteDevice* instance) {
    return instance->attack;
}
bool subbrute_device_is_worker_running(SubBruteDevice* instance) {
    return instance->worker_running;
}
uint64_t subbrute_device_get_step(SubBruteDevice* instance) {
    return instance->key_index;
}
const char* subbrute_device_get_file_key(SubBruteDevice* instance) {
    return instance->file_key;
}
uint64_t subbrute_device_add_step(SubBruteDevice* instance, int8_t step) {
    if(!subbrute_device_can_manual_transmit(instance)) {
        return instance->key_index;
    }
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
            uint64_t value = ((instance->key_index - step) + instance->max_value);
            if(value == instance->max_value) {
                instance->key_index = value;
            } else {
                instance->key_index = value % instance->max_value;
            }
        }
    }

    return instance->key_index;
}
void subbrute_device_set_load_index(SubBruteDevice* instance, uint64_t load_index) {
    instance->load_index = load_index;
}
void subbrute_device_reset_step(SubBruteDevice* instance) {
    instance->key_index = 0x00;
}
void subbrute_device_subghz_transmit(SubBruteDevice* instance, FlipperFormat* flipper_format) {
    instance->transmitter = subghz_transmitter_alloc_init(
        instance->environment, subbrute_protocol_name(instance->attack));
    subghz_transmitter_deserialize(instance->transmitter, flipper_format);
    furi_hal_subghz_reset();
    furi_hal_subghz_load_preset(instance->protocol_info->preset);
    furi_hal_subghz_set_frequency_and_path(instance->protocol_info->preset);

    furi_hal_subghz_start_async_tx(subghz_transmitter_yield, instance->transmitter);

    while(!furi_hal_subghz_is_async_tx_complete()) {
        furi_delay_ms(SUBBRUTE_TX_TIMEOUT);
    }
    furi_hal_subghz_stop_async_tx();

    furi_hal_subghz_set_path(FuriHalSubGhzPathIsolate);
    furi_hal_subghz_sleep();
    subghz_transmitter_free(instance->transmitter);
    instance->transmitter = NULL;
}

bool subbrute_device_transmit_current_key(SubBruteDevice* instance) {
    furi_assert(instance);

    if(instance->worker_running) {
        FURI_LOG_W(TAG, "Worker in running state!");
        return false;
    }
    if(instance->state != SubBruteDeviceStateReady &&
       instance->state != SubBruteDeviceStateFinished) {
        FURI_LOG_W(TAG, "Invalid state for running worker! State: %d", instance->state);
        return false;
    }

    uint32_t ticks = furi_get_tick();
    if((ticks - instance->last_time_tx_data) < SUBBRUTE_MANUAL_TRANSMIT_INTERVAL) {
#if FURI_DEBUG
        FURI_LOG_D(TAG, "Need to wait, current: %d", ticks - instance->last_time_tx_data);
#endif
        return false;
    }

    instance->last_time_tx_data = ticks;
    FlipperFormat* flipper_format = flipper_format_string_alloc();

    if(!subbrute_device_create_packet_parsed(instance, flipper_format, instance->key_index, true)) {
        FURI_LOG_W(TAG, "Error creating packet! EXIT");
        return false;
    }
    subbrute_device_subghz_transmit(instance, flipper_format);

    flipper_format_free(flipper_format);

    return true;
}

void subbrute_device_set_callback(
    SubBruteDevice* instance,
    SubBruteDeviceWorkerCallback callback,
    void* context) {
    furi_assert(instance);

    instance->callback = callback;
    instance->context = context;
}

bool subbrute_device_can_manual_transmit(SubBruteDevice* instance) {
    furi_assert(instance);

    return !instance->worker_running && instance->state != SubBruteDeviceStateIDLE &&
           instance->state != SubBruteDeviceStateTx &&
           ((furi_get_tick() - instance->last_time_tx_data) > SUBBRUTE_MANUAL_TRANSMIT_INTERVAL);
}

bool subbrute_device_save_file(SubBruteDevice* instance, const char* dev_file_name) {
    furi_assert(instance);

    if(instance->state != SubBruteDeviceStateReady &&
       instance->state != SubBruteDeviceStateFinished) {
        FURI_LOG_W(TAG, "Worker is not set to running state!");
        return false;
    }

#ifdef FURI_DEBUG
    FURI_LOG_D(TAG, "subbrute_device_save_file: %s", dev_file_name);
#endif

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);

    bool result = false;
    do {
        if(!flipper_format_file_open_always(file, dev_file_name)) {
            break;
        }

        if(!subbrute_device_create_packet_parsed(instance, file, instance->key_index, false)) {
            FURI_LOG_E(TAG, "subbrute_device_create_packet_parsed failed!");
            break;
        }

        result = true;
    } while(false);

    if(!result) {
        FURI_LOG_E(TAG, "flipper_format_file_open_always failed!");
    }

    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);

    return result;
}

bool subbrute_device_create_packet_parsed(
    SubBruteDevice* instance,
    FlipperFormat* flipper_format,
    uint64_t step,
    bool small) {
    furi_assert(instance);

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

    Stream* stream = flipper_format_get_raw_stream(flipper_format);
    stream_clean(stream);

    if(small) {
        if(instance->protocol_info->te) {
            stream_write_format(
                stream,
                subbrute_key_small_with_tail,
                instance->protocol_info->bits,
                string_get_cstr(candidate),
                instance->protocol_info->te,
                instance->protocol_info->repeat);
        } else {
            stream_write_format(
                stream,
                subbrute_key_small_no_tail,
                instance->protocol_info->bits,
                string_get_cstr(candidate),
                instance->protocol_info->repeat);
        }
    } else {
        if(instance->protocol_info->te) {
            stream_write_format(
                stream,
                subbrute_key_file_key_with_tail,
                instance->file_template,
                string_get_cstr(candidate),
                instance->protocol_info->te,
                instance->protocol_info->repeat);
        } else {
            stream_write_format(
                stream,
                subbrute_key_file_key,
                instance->file_template,
                string_get_cstr(candidate),
                instance->protocol_info->repeat);
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

    if(type != SubBruteAttackLoadFile) {
        subbrute_device_free_protocol_info(instance);
        instance->protocol_info = subbrute_protocol(type);
    }

    // For non-file types we didn't set SubGhzProtocolDecoderBase
    instance->receiver = subghz_receiver_alloc_init(instance->environment);
    subghz_receiver_set_filter(instance->receiver, SubGhzProtocolFlag_Decodable);
    furi_hal_subghz_reset();

    uint8_t protocol_check_result = SubBruteFileResultProtocolNotFound;
    if(type != SubBruteAttackLoadFile) {
        instance->decoder_result = subghz_receiver_search_decoder_base_by_name(
            instance->receiver, subbrute_protocol_file(instance->protocol_info->file));

        if(!instance->decoder_result ||
           instance->decoder_result->protocol->type == SubGhzProtocolTypeDynamic) {
            FURI_LOG_E(TAG, "Can't load SubGhzProtocolDecoderBase in phase non-file decoder set");
        } else {
            protocol_check_result = SubBruteFileResultOk;
        }
    } else {
        // And here we need to set preset enum
        protocol_check_result = SubBruteFileResultOk;
    }

    subghz_receiver_free(instance->receiver);
    instance->receiver = NULL;

    if(protocol_check_result != SubBruteFileResultOk) {
        return SubBruteFileResultProtocolNotFound;
    }

    // Calc max value
    if(instance->attack == SubBruteAttackLoadFile) {
        instance->max_value = 0xFF;
    } else {
        string_t max_value_s;
        string_init(max_value_s);
        for(uint8_t i = 0; i < instance->protocol_info->bits; i++) {
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
        instance->protocol_info->frequency,
        subbrute_protocol_preset(instance->protocol_info->preset),
        subbrute_protocol_file(instance->protocol_info->file),
        instance->protocol_info->bits);
#ifdef FURI_DEBUG
    FURI_LOG_D(
        TAG, "tail: %d, file_template: %s", instance->protocol_info->te, instance->file_template);
#endif

    // Init payload
    FlipperFormat* flipper_format = flipper_format_string_alloc();
    if(subbrute_device_create_packet_parsed(instance, flipper_format, instance->key_index, false)) {
        instance->state = SubBruteDeviceStateReady;
        subbrute_device_send_callback(instance);
    }
    flipper_format_free(flipper_format);

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
            instance->protocol_info->frequency = temp_data32;
            if(!furi_hal_subghz_is_tx_allowed(instance->protocol_info->frequency)) {
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
            instance->protocol_info->preset = subbrute_protocol_convert_preset(temp_str);
        }

        const char* protocol_file = NULL;
        // Protocol
        if(!flipper_format_read_string(fff_data_file, "Protocol", temp_str)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            result = SubBruteFileResultMissingProtocol;
            break;
        } else {
            instance->protocol_info->file = subbrute_protocol_file_protocol_name(temp_str);
            protocol_file = subbrute_protocol_file(instance->protocol_info->file);
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
            instance->protocol_info->bits = temp_data32;
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Bit: %d", instance->protocol_info->bits);
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
            instance->protocol_info->te = temp_data32 != 0;
        }

        // Repeat
        if(flipper_format_read_uint32(fff_data_file, "Repeat", &temp_data32, 1)) {
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Repeat: %d", temp_data32);
#endif
            instance->protocol_info->repeat = (uint8_t)temp_data32;
        } else {
#ifdef FURI_DEBUG
            FURI_LOG_D(TAG, "Repeat: 3 (default)");
#endif
            instance->protocol_info->repeat = 3;
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

    if(default_attack != SubBruteAttackLoadFile) {
        memset(instance->file_key, 0, sizeof(instance->file_key));

        instance->max_value = (uint64_t)0x00;
    }
}

void subbrute_device_send_callback(SubBruteDevice* instance) {
    if(instance->callback != NULL) {
        instance->callback(instance->context, instance->state);
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