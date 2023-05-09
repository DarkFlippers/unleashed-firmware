#include "subghz_txrx.h"
#include <lib/subghz/protocols/protocol_items.h>

#define TAG "SubGhz"

struct SubGhzTxRx {
    SubGhzWorker* worker;

    SubGhzEnvironment* environment;
    SubGhzReceiver* receiver;
    SubGhzTransmitter* transmitter;
    SubGhzProtocolDecoderBase* decoder_result;
    FlipperFormat* fff_data;

    SubGhzRadioPreset* preset;
    SubGhzSetting* setting;

    uint8_t hopper_timeout;
    uint8_t hopper_idx_frequency;
    bool load_database;
    SubGhzHopperState hopper_state;

    SubGhzTxRxState txrx_state;
    SubGhzSpeakerState speaker_state;

    SubGhzTxRxNeedSaveCallback need_save_callback;
    void* need_save_context;

    bool debug_pin_state;
};

SubGhzTxRx* subghz_txrx_alloc() {
    SubGhzTxRx* instance = malloc(sizeof(SubGhzTxRx));
    instance->setting = subghz_setting_alloc();
    subghz_setting_load(instance->setting, EXT_PATH("subghz/assets/setting_user"));

    instance->preset = malloc(sizeof(SubGhzRadioPreset));
    instance->preset->name = furi_string_alloc();
    subghz_txrx_set_preset(
        instance, "AM650", subghz_setting_get_default_frequency(instance->setting), NULL, 0);

    instance->txrx_state = SubGhzTxRxStateSleep;

    subghz_txrx_hopper_set_state(instance, SubGhzHopperStateOFF);
    subghz_txrx_speaker_set_state(instance, SubGhzSpeakerStateDisable);
    subghz_txrx_set_debug_pin_state(instance, false);

    instance->worker = subghz_worker_alloc();
    instance->fff_data = flipper_format_string_alloc();

    instance->environment = subghz_environment_alloc();
    instance->load_database = subghz_environment_load_keystore(
        instance->environment, EXT_PATH("subghz/assets/keeloq_mfcodes"));
    subghz_environment_load_keystore(
        instance->environment, EXT_PATH("subghz/assets/keeloq_mfcodes_user"));
    subghz_environment_set_came_atomo_rainbow_table_file_name(
        instance->environment, EXT_PATH("subghz/assets/came_atomo"));
    subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
        instance->environment, EXT_PATH("subghz/assets/alutech_at_4n"));
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        instance->environment, EXT_PATH("subghz/assets/nice_flor_s"));
    subghz_environment_set_protocol_registry(
        instance->environment, (void*)&subghz_protocol_registry);
    instance->receiver = subghz_receiver_alloc_init(instance->environment);

    subghz_worker_set_overrun_callback(
        instance->worker, (SubGhzWorkerOverrunCallback)subghz_receiver_reset);
    subghz_worker_set_pair_callback(
        instance->worker, (SubGhzWorkerPairCallback)subghz_receiver_decode);
    subghz_worker_set_context(instance->worker, instance->receiver);

    return instance;
}

void subghz_txrx_free(SubGhzTxRx* instance) {
    furi_assert(instance);

    subghz_worker_free(instance->worker);
    subghz_receiver_free(instance->receiver);
    subghz_environment_free(instance->environment);
    flipper_format_free(instance->fff_data);
    furi_string_free(instance->preset->name);
    subghz_setting_free(instance->setting);
    free(instance->preset);
    free(instance);
}

bool subghz_txrx_is_load_database(SubGhzTxRx* instance) {
    furi_assert(instance);
    return instance->load_database;
}

void subghz_txrx_set_preset(
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    uint8_t* preset_data,
    size_t preset_data_size) {
    furi_assert(instance);
    furi_string_set(instance->preset->name, preset_name);
    instance->preset->frequency = frequency;
    instance->preset->data = preset_data;
    instance->preset->data_size = preset_data_size;
}

const char* subghz_txrx_get_name_preset(SubGhzTxRx* instance, const char* preset) {
    UNUSED(instance);
    const char* preset_name = NULL;
    if(!strcmp(preset, "FuriHalSubGhzPresetOok270Async")) {
        preset_name = "AM270";
    } else if(!strcmp(preset, "FuriHalSubGhzPresetOok650Async")) {
        preset_name = "AM650";
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev238Async")) {
        preset_name = "FM238";
    } else if(!strcmp(preset, "FuriHalSubGhzPreset2FSKDev476Async")) {
        preset_name = "FM476";
    } else if(!strcmp(preset, "FuriHalSubGhzPresetCustom")) {
        preset_name = "CUSTOM";
    } else {
        FURI_LOG_E(TAG, "Unknown preset");
    }
    return preset_name;
}

SubGhzRadioPreset subghz_txrx_get_preset(SubGhzTxRx* instance) {
    furi_assert(instance);
    return *instance->preset;
}

void subghz_txrx_get_frequency_modulation(
    SubGhzTxRx* instance,
    FuriString* frequency,
    FuriString* modulation,
    bool long_name) {
    furi_assert(instance);
    if(frequency != NULL) {
        furi_string_printf(
            frequency,
            "%03ld.%02ld",
            instance->preset->frequency / 1000000 % 1000,
            instance->preset->frequency / 10000 % 100);
    }
    if(modulation != NULL) {
        if(long_name) {
            furi_string_printf(modulation, "%s", furi_string_get_cstr(instance->preset->name));
        } else {
            furi_string_printf(modulation, "%.2s", furi_string_get_cstr(instance->preset->name));
        }
    }
}

static void subghz_txrx_begin(SubGhzTxRx* instance, uint8_t* preset_data) {
    furi_assert(instance);
    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_custom_preset(preset_data);
    furi_hal_gpio_init(furi_hal_subghz.cc1101_g0_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);
    instance->txrx_state = SubGhzTxRxStateIDLE;
}

static uint32_t subghz_txrx_rx(SubGhzTxRx* instance, uint32_t frequency) {
    furi_assert(instance);
    if(!furi_hal_subghz_is_frequency_valid(frequency)) {
        furi_crash("SubGhz: Incorrect RX frequency.");
    }
    furi_assert(
        instance->txrx_state != SubGhzTxRxStateRx && instance->txrx_state != SubGhzTxRxStateSleep);

    furi_hal_subghz_idle();
    uint32_t value = furi_hal_subghz_set_frequency_and_path(frequency);
    furi_hal_gpio_init(furi_hal_subghz.cc1101_g0_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);
    furi_hal_subghz_flush_rx();
    subghz_txrx_speaker_on(instance);
    furi_hal_subghz_rx();

    furi_hal_subghz_start_async_rx(subghz_worker_rx_callback, instance->worker);
    subghz_worker_start(instance->worker);
    instance->txrx_state = SubGhzTxRxStateRx;
    return value;
}

static void subghz_txrx_idle(SubGhzTxRx* instance) {
    furi_assert(instance);
    furi_assert(instance->txrx_state != SubGhzTxRxStateSleep);
    furi_hal_subghz_idle();
    subghz_txrx_speaker_off(instance);
    instance->txrx_state = SubGhzTxRxStateIDLE;
}

static void subghz_txrx_rx_end(SubGhzTxRx* instance) {
    furi_assert(instance);
    furi_assert(instance->txrx_state == SubGhzTxRxStateRx);

    if(subghz_worker_is_running(instance->worker)) {
        subghz_worker_stop(instance->worker);
        furi_hal_subghz_stop_async_rx();
    }
    furi_hal_subghz_idle();
    subghz_txrx_speaker_off(instance);
    instance->txrx_state = SubGhzTxRxStateIDLE;
}

void subghz_txrx_sleep(SubGhzTxRx* instance) {
    furi_assert(instance);
    furi_hal_subghz_sleep();
    instance->txrx_state = SubGhzTxRxStateSleep;
}

static bool subghz_txrx_tx(SubGhzTxRx* instance, uint32_t frequency) {
    furi_assert(instance);
    if(!furi_hal_subghz_is_frequency_valid(frequency)) {
        furi_crash("SubGhz: Incorrect TX frequency.");
    }
    furi_assert(instance->txrx_state != SubGhzTxRxStateSleep);
    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency_and_path(frequency);
    furi_hal_gpio_write(furi_hal_subghz.cc1101_g0_pin, false);
    furi_hal_gpio_init(
        furi_hal_subghz.cc1101_g0_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    bool ret = furi_hal_subghz_tx();
    if(ret) {
        subghz_txrx_speaker_on(instance);
        instance->txrx_state = SubGhzTxRxStateTx;
    }
    return ret;
}

SubGhzTxRxStartTxState subghz_txrx_tx_start(SubGhzTxRx* instance, FlipperFormat* flipper_format) {
    furi_assert(instance);
    furi_assert(flipper_format);

    subghz_txrx_stop(instance);

    SubGhzTxRxStartTxState ret = SubGhzTxRxStartTxStateErrorParserOthers;
    FuriString* temp_str = furi_string_alloc();
    uint32_t repeat = 200;
    do {
        if(!flipper_format_rewind(flipper_format)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }
        if(!flipper_format_read_string(flipper_format, "Protocol", temp_str)) {
            FURI_LOG_E(TAG, "Missing Protocol");
            break;
        }
        if(!flipper_format_insert_or_update_uint32(flipper_format, "Repeat", &repeat, 1)) {
            FURI_LOG_E(TAG, "Unable Repeat");
            break;
        }

        instance->transmitter =
            subghz_transmitter_alloc_init(instance->environment, furi_string_get_cstr(temp_str));

        if(instance->transmitter) {
            if(subghz_transmitter_deserialize(instance->transmitter, flipper_format) ==
               SubGhzProtocolStatusOk) {
                if(strcmp(furi_string_get_cstr(instance->preset->name), "") != 0) {
                    subghz_txrx_begin(
                        instance,
                        subghz_setting_get_preset_data_by_name(
                            instance->setting, furi_string_get_cstr(instance->preset->name)));
                    if(instance->preset->frequency) {
                        if(!subghz_txrx_tx(instance, instance->preset->frequency)) {
                            FURI_LOG_E(TAG, "Only Rx");
                            ret = SubGhzTxRxStartTxStateErrorOnlyRx;
                        }
                    } else {
                        ret = SubGhzTxRxStartTxStateErrorParserOthers;
                    }
                } else {
                    FURI_LOG_E(
                        TAG,
                        "Unknown name preset \" %s \"",
                        furi_string_get_cstr(instance->preset->name));
                    ret = SubGhzTxRxStartTxStateErrorParserOthers;
                }
                if(ret == SubGhzTxRxStartTxStateOk) {
                    //Start TX
                    furi_hal_subghz_start_async_tx(
                        subghz_transmitter_yield, instance->transmitter);
                }
            } else {
                ret = SubGhzTxRxStartTxStateErrorParserOthers;
            }
        }
        if(ret != SubGhzTxRxStartTxStateOk) {
            subghz_transmitter_free(instance->transmitter);
            if(instance->txrx_state != SubGhzTxRxStateIDLE) {
                subghz_txrx_idle(instance);
            }
        }

    } while(false);
    furi_string_free(temp_str);
    return ret;
}

void subghz_txrx_rx_start(SubGhzTxRx* instance) {
    furi_assert(instance);
    subghz_txrx_stop(instance);
    subghz_txrx_begin(
        instance,
        subghz_setting_get_preset_data_by_name(
            subghz_txrx_get_setting(instance), furi_string_get_cstr(instance->preset->name)));
    subghz_txrx_rx(instance, instance->preset->frequency);
}

void subghz_txrx_need_save_callback_set(
    SubGhzTxRx* instance,
    SubGhzTxRxNeedSaveCallback callback,
    void* context) {
    furi_assert(instance);
    instance->need_save_callback = callback;
    instance->need_save_context = context;
}

static void subghz_txrx_tx_stop(SubGhzTxRx* instance) {
    furi_assert(instance);
    furi_assert(instance->txrx_state == SubGhzTxRxStateTx);
    //Stop TX
    furi_hal_subghz_stop_async_tx();
    subghz_transmitter_stop(instance->transmitter);
    subghz_transmitter_free(instance->transmitter);

    //if protocol dynamic then we save the last upload
    if(instance->decoder_result->protocol->type == SubGhzProtocolTypeDynamic) {
        if(instance->need_save_callback) {
            instance->need_save_callback(instance->need_save_context);
        }
    }
    subghz_txrx_idle(instance);
    subghz_txrx_speaker_off(instance);
    //Todo: Show message
    // notification_message(notifications, &sequence_reset_red);
}

FlipperFormat* subghz_txtx_get_fff_data(SubGhzTxRx* instance) {
    furi_assert(instance);
    return instance->fff_data;
}

SubGhzSetting* subghz_txrx_get_setting(SubGhzTxRx* instance) {
    furi_assert(instance);
    return instance->setting;
}

void subghz_txrx_stop(SubGhzTxRx* instance) {
    furi_assert(instance);

    switch(instance->txrx_state) {
    case SubGhzTxRxStateTx:
        subghz_txrx_tx_stop(instance);
        subghz_txrx_speaker_unmute(instance);
        break;
    case SubGhzTxRxStateRx:
        subghz_txrx_rx_end(instance);
        subghz_txrx_speaker_mute(instance);
        break;

    default:
        break;
    }
}

void subghz_txrx_hopper_update(SubGhzTxRx* instance) {
    furi_assert(instance);

    switch(instance->hopper_state) {
    case SubGhzHopperStateOFF:
    case SubGhzHopperStatePause:
        return;
    case SubGhzHopperStateRSSITimeOut:
        if(instance->hopper_timeout != 0) {
            instance->hopper_timeout--;
            return;
        }
        break;
    default:
        break;
    }
    float rssi = -127.0f;
    if(instance->hopper_state != SubGhzHopperStateRSSITimeOut) {
        // See RSSI Calculation timings in CC1101 17.3 RSSI
        rssi = furi_hal_subghz_get_rssi();

        // Stay if RSSI is high enough
        if(rssi > -90.0f) {
            instance->hopper_timeout = 10;
            instance->hopper_state = SubGhzHopperStateRSSITimeOut;
            return;
        }
    } else {
        instance->hopper_state = SubGhzHopperStateRunning;
    }
    // Select next frequency
    if(instance->hopper_idx_frequency <
       subghz_setting_get_hopper_frequency_count(instance->setting) - 1) {
        instance->hopper_idx_frequency++;
    } else {
        instance->hopper_idx_frequency = 0;
    }

    if(instance->txrx_state == SubGhzTxRxStateRx) {
        subghz_txrx_rx_end(instance);
    };
    if(instance->txrx_state == SubGhzTxRxStateIDLE) {
        subghz_receiver_reset(instance->receiver);
        instance->preset->frequency =
            subghz_setting_get_hopper_frequency(instance->setting, instance->hopper_idx_frequency);
        subghz_txrx_rx(instance, instance->preset->frequency);
    }
}

SubGhzHopperState subghz_txrx_hopper_get_state(SubGhzTxRx* instance) {
    furi_assert(instance);
    return instance->hopper_state;
}

void subghz_txrx_hopper_set_state(SubGhzTxRx* instance, SubGhzHopperState state) {
    furi_assert(instance);
    instance->hopper_state = state;
}

void subghz_txrx_hopper_remove_pause(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->hopper_state == SubGhzHopperStatePause) {
        instance->hopper_state = SubGhzHopperStateRunning;
    }
}

void subghz_txrx_hopper_pause(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->hopper_state == SubGhzHopperStateRunning) {
        instance->hopper_state = SubGhzHopperStatePause;
    }
}

void subghz_txrx_speaker_on(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->debug_pin_state) {
        furi_hal_subghz_set_async_mirror_pin(&gpio_ibutton);
    }

    if(instance->speaker_state == SubGhzSpeakerStateEnable) {
        if(furi_hal_speaker_acquire(30)) {
            if(!instance->debug_pin_state) {
                furi_hal_subghz_set_async_mirror_pin(&gpio_speaker);
            }
        } else {
            instance->speaker_state = SubGhzSpeakerStateDisable;
        }
    }
}

void subghz_txrx_speaker_off(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->debug_pin_state) {
        furi_hal_subghz_set_async_mirror_pin(NULL);
    }
    if(instance->speaker_state != SubGhzSpeakerStateDisable) {
        if(furi_hal_speaker_is_mine()) {
            if(!instance->debug_pin_state) {
                furi_hal_subghz_set_async_mirror_pin(NULL);
            }
            furi_hal_speaker_release();
            if(instance->speaker_state == SubGhzSpeakerStateShutdown)
                instance->speaker_state = SubGhzSpeakerStateDisable;
        }
    }
}

void subghz_txrx_speaker_mute(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->debug_pin_state) {
        furi_hal_subghz_set_async_mirror_pin(NULL);
    }
    if(instance->speaker_state == SubGhzSpeakerStateEnable) {
        if(furi_hal_speaker_is_mine()) {
            if(!instance->debug_pin_state) {
                furi_hal_subghz_set_async_mirror_pin(NULL);
            }
        }
    }
}

void subghz_txrx_speaker_unmute(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->debug_pin_state) {
        furi_hal_subghz_set_async_mirror_pin(&gpio_ibutton);
    }
    if(instance->speaker_state == SubGhzSpeakerStateEnable) {
        if(furi_hal_speaker_is_mine()) {
            if(!instance->debug_pin_state) {
                furi_hal_subghz_set_async_mirror_pin(&gpio_speaker);
            }
        }
    }
}

void subghz_txrx_speaker_set_state(SubGhzTxRx* instance, SubGhzSpeakerState state) {
    furi_assert(instance);
    instance->speaker_state = state;
}

SubGhzSpeakerState subghz_txrx_speaker_get_state(SubGhzTxRx* instance) {
    furi_assert(instance);
    return instance->speaker_state;
}

bool subghz_txrx_load_decoder_by_name_protocol(SubGhzTxRx* instance, const char* name_protocol) {
    furi_assert(instance);
    furi_assert(name_protocol);
    bool res = false;
    instance->decoder_result =
        subghz_receiver_search_decoder_base_by_name(instance->receiver, name_protocol);
    if(instance->decoder_result) {
        res = true;
    }
    return res;
}

SubGhzProtocolDecoderBase* subghz_txrx_get_decoder(SubGhzTxRx* instance) {
    furi_assert(instance);
    return instance->decoder_result;
}

bool subghz_txrx_protocol_is_preserved(SubGhzTxRx* instance) {
    furi_assert(instance);
    return (
        (instance->decoder_result->protocol->flag & SubGhzProtocolFlag_Save) ==
        SubGhzProtocolFlag_Save);
}

bool subghz_txrx_protocol_is_send(SubGhzTxRx* instance, bool check_type) {
    furi_assert(instance);
    if(check_type) {
        return (
            ((instance->decoder_result->protocol->flag & SubGhzProtocolFlag_Send) ==
             SubGhzProtocolFlag_Send) &&
            instance->decoder_result->protocol->encoder->deserialize &&
            instance->decoder_result->protocol->type == SubGhzProtocolTypeStatic);
    }
    return (
        ((instance->decoder_result->protocol->flag & SubGhzProtocolFlag_Send) ==
         SubGhzProtocolFlag_Send) &&
        instance->decoder_result->protocol->encoder->deserialize);
}

void subghz_txrx_receiver_set_filter(SubGhzTxRx* instance, SubGhzProtocolFlag filter) {
    furi_assert(instance);
    subghz_receiver_set_filter(instance->receiver, filter);
}

void subghz_txrx_set_rx_calback(
    SubGhzTxRx* instance,
    SubGhzReceiverCallback callback,
    void* context) {
    subghz_receiver_set_rx_callback(instance->receiver, callback, context);
}

void subghz_txrx_set_raw_file_encoder_worker_set_callback_end(
    SubGhzTxRx* instance,
    SubGhzProtocolEncoderRAWCallbackEnd callback,
    void* context) {
    subghz_protocol_raw_file_encoder_worker_set_callback_end(
        (SubGhzProtocolEncoderRAW*)subghz_transmitter_get_protocol_instance(instance->transmitter),
        callback,
        context);
}

void subghz_txrx_set_debug_pin_state(SubGhzTxRx* instance, bool state) {
    furi_assert(instance);
    instance->debug_pin_state = state;
}

bool subghz_txrx_get_debug_pin_state(SubGhzTxRx* instance) {
    furi_assert(instance);
    return instance->debug_pin_state;
}

SubGhzReceiver* subghz_txrx_get_receiver(SubGhzTxRx* instance) {
    furi_assert(instance);
    return instance->receiver;
}

//#############Create  new Key##############
#include <lib/subghz/protocols/protocol_items.h>
#include <lib/subghz/protocols/keeloq.h>
#include <lib/subghz/protocols/secplus_v1.h>
#include <lib/subghz/protocols/secplus_v2.h>
#include <lib/subghz/protocols/nice_flor_s.h>

#include <flipper_format/flipper_format_i.h>
#include <lib/toolbox/stream/stream.h>
#include <lib/subghz/protocols/raw.h>

bool subghz_txrx_gen_data_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit) {
    furi_assert(context);
    SubGhzTxRx* instance = context;

    bool res = false;

    subghz_txrx_set_preset(instance, preset_name, frequency, NULL, 0);
    instance->decoder_result =
        subghz_receiver_search_decoder_base_by_name(instance->receiver, protocol_name);

    if(instance->decoder_result == NULL) {
        //TODO: Error
        // furi_string_set(error_str, "Protocol not\nfound!");
        // scene_manager_next_scene(scene_manager, SubGhzSceneShowErrorSub);
        return false;
    }

    do {
        Stream* fff_data_stream = flipper_format_get_raw_stream(instance->fff_data);
        stream_clean(fff_data_stream);
        if(subghz_protocol_decoder_base_serialize(
               instance->decoder_result, instance->fff_data, instance->preset) !=
           SubGhzProtocolStatusOk) {
            FURI_LOG_E(TAG, "Unable to serialize");
            break;
        }
        if(!flipper_format_update_uint32(instance->fff_data, "Bit", &bit, 1)) {
            FURI_LOG_E(TAG, "Unable to update Bit");
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (key >> (i * 8)) & 0xFF;
        }
        if(!flipper_format_update_hex(instance->fff_data, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Key");
            break;
        }
        res = true;
    } while(false);
    return res;
}

bool subghz_txrx_gen_data_protocol_and_te(
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit,
    uint32_t te) {
    furi_assert(instance);
    bool ret = false;
    if(subghz_txrx_gen_data_protocol(instance, preset_name, frequency, protocol_name, key, bit)) {
        if(!flipper_format_update_uint32(instance->fff_data, "TE", (uint32_t*)&te, 1)) {
            FURI_LOG_E(TAG, "Unable to update Te");
        } else {
            ret = true;
        }
    }
    return ret;
}

bool subghz_scene_set_type_submenu_gen_data_keeloq( //TODO rename
    SubGhzTxRx* txrx,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name) {
    furi_assert(txrx);

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_KEELOQ_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_keeloq_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                manufacture_name,
                                txrx->preset)) {
        flipper_format_write_string_cstr(txrx->fff_data, "Manufacture", manufacture_name);
        res = true;
    }
    subghz_transmitter_free(txrx->transmitter);
    return res;
}

bool subghz_scene_set_type_submenu_gen_data_keeloq_bft( //TODO rename
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint32_t seed,
    const char* manufacture_name) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_KEELOQ_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_keeloq_bft_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                seed,
                                manufacture_name,
                                txrx->preset)) {
        res = true;
    }

    if(res) {
        uint8_t seed_data[sizeof(uint32_t)] = {0};
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            seed_data[sizeof(uint32_t) - i - 1] = (seed >> i * 8) & 0xFF;
        }

        flipper_format_write_hex(txrx->fff_data, "Seed", seed_data, sizeof(uint32_t));

        flipper_format_write_string_cstr(txrx->fff_data, "Manufacture", "BFT");
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_scene_set_type_submenu_gen_data_nice_flor( //TODO rename
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    bool nice_one) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_nice_flor_s_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                txrx->preset,
                                nice_one)) {
        res = true;
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_scene_set_type_submenu_gen_data_faac_slh( //TODO rename
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint32_t seed,
    const char* manufacture_name) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_FAAC_SLH_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_faac_slh_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                seed,
                                manufacture_name,
                                txrx->preset)) {
        res = true;
    }

    if(res) {
        uint8_t seed_data[sizeof(uint32_t)] = {0};
        for(size_t i = 0; i < sizeof(uint32_t); i++) {
            seed_data[sizeof(uint32_t) - i - 1] = (seed >> i * 8) & 0xFF;
        }

        flipper_format_write_hex(txrx->fff_data, "Seed", seed_data, sizeof(uint32_t));
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_scene_set_type_submenu_gen_data_alutech_at_4n( //TODO rename
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_ALUTECH_AT_4N_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_alutech_at_4n_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                txrx->preset)) {
        res = true;
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_scene_set_type_submenu_gen_data_somfy_telis( //TODO rename
    void* context,
    const char* preset_name,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt) {
    SubGhzTxRx* txrx = context;

    bool res = false;

    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_SOMFY_TELIS_NAME);
    subghz_txrx_set_preset(txrx, preset_name, frequency, NULL, 0);

    if(txrx->transmitter && subghz_protocol_somfy_telis_create_data(
                                subghz_transmitter_get_protocol_instance(txrx->transmitter),
                                txrx->fff_data,
                                serial,
                                btn,
                                cnt,
                                txrx->preset)) {
        res = true;
    }

    subghz_transmitter_free(txrx->transmitter);

    return res;
}

bool subghz_txrx_gen_secplus_v2_protocol(
    SubGhzTxRx* txrx,
    const char* name_preset,
    uint32_t frequency,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt) {
    furi_assert(txrx);

    bool ret = false;
    txrx->transmitter =
        subghz_transmitter_alloc_init(txrx->environment, SUBGHZ_PROTOCOL_SECPLUS_V2_NAME);
    subghz_txrx_set_preset(txrx, name_preset, frequency, NULL, 0);
    if(txrx->transmitter) {
        subghz_protocol_secplus_v2_create_data(
            subghz_transmitter_get_protocol_instance(txrx->transmitter),
            txrx->fff_data,
            serial,
            btn,
            cnt,
            txrx->preset);
        ret = true;
    }
    return ret;
}

bool subghz_txrx_gen_secplus_v1_protocol(
    SubGhzTxRx* txrx,
    const char* name_preset,
    uint32_t frequency) {
    furi_assert(txrx);

    bool ret = false;
    uint32_t serial = (uint32_t)rand();
    while(!subghz_protocol_secplus_v1_check_fixed(serial)) {
        serial = (uint32_t)rand();
    }
    if(subghz_txrx_gen_data_protocol(
           txrx,
           name_preset,
           frequency,
           SUBGHZ_PROTOCOL_SECPLUS_V1_NAME,
           (uint64_t)serial << 32 | 0xE6000000,
           42)) {
        ret = true;
    }
    return ret;
}