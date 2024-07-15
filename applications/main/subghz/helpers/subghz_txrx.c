#include "subghz_txrx_i.h" // IWYU pragma: keep

#include <lib/subghz/protocols/protocol_items.h>
#include <applications/drivers/subghz/cc1101_ext/cc1101_ext_interconnect.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>

#define TAG "SubGhz"

static void subghz_txrx_radio_device_power_on(SubGhzTxRx* instance) {
    UNUSED(instance);
    uint8_t attempts = 5;
    while(--attempts > 0) {
        if(furi_hal_power_enable_otg()) break;
    }
    if(attempts == 0) {
        if(furi_hal_power_get_usb_voltage() < 4.5f) {
            FURI_LOG_E(
                TAG,
                "Error power otg enable. BQ2589 check otg fault = %d",
                furi_hal_power_check_otg_fault() ? 1 : 0);
        }
    }
}

static void subghz_txrx_radio_device_power_off(SubGhzTxRx* instance) {
    UNUSED(instance);
    if(furi_hal_power_is_otg_enabled()) furi_hal_power_disable_otg();
}

SubGhzTxRx* subghz_txrx_alloc(void) {
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

    instance->worker = subghz_worker_alloc();
    instance->fff_data = flipper_format_string_alloc();

    instance->environment = subghz_environment_alloc();
    instance->is_database_loaded =
        subghz_environment_load_keystore(instance->environment, SUBGHZ_KEYSTORE_DIR_NAME);
    subghz_environment_load_keystore(instance->environment, SUBGHZ_KEYSTORE_DIR_USER_NAME);
    subghz_environment_set_came_atomo_rainbow_table_file_name(
        instance->environment, SUBGHZ_CAME_ATOMO_DIR_NAME);
    subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
        instance->environment, SUBGHZ_ALUTECH_AT_4N_DIR_NAME);
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        instance->environment, SUBGHZ_NICE_FLOR_S_DIR_NAME);
    subghz_environment_set_protocol_registry(
        instance->environment, (void*)&subghz_protocol_registry);
    instance->receiver = subghz_receiver_alloc_init(instance->environment);

    subghz_worker_set_overrun_callback(
        instance->worker, (SubGhzWorkerOverrunCallback)subghz_receiver_reset);
    subghz_worker_set_pair_callback(
        instance->worker, (SubGhzWorkerPairCallback)subghz_receiver_decode);
    subghz_worker_set_context(instance->worker, instance->receiver);

    //set default device External
    subghz_devices_init();
    instance->radio_device_type = SubGhzRadioDeviceTypeInternal;
    instance->radio_device_type =
        subghz_txrx_radio_device_set(instance, SubGhzRadioDeviceTypeExternalCC1101);

    return instance;
}

void subghz_txrx_free(SubGhzTxRx* instance) {
    furi_assert(instance);

    if(instance->radio_device_type != SubGhzRadioDeviceTypeInternal) {
        subghz_txrx_radio_device_power_off(instance);
        subghz_devices_end(instance->radio_device);
    }

    subghz_devices_deinit();

    subghz_worker_free(instance->worker);
    subghz_receiver_free(instance->receiver);
    subghz_environment_free(instance->environment);
    flipper_format_free(instance->fff_data);
    furi_string_free(instance->preset->name);
    subghz_setting_free(instance->setting);

    free(instance->preset);
    free(instance);
}

bool subghz_txrx_is_database_loaded(SubGhzTxRx* instance) {
    furi_assert(instance);
    return instance->is_database_loaded;
}

void subghz_txrx_set_preset(
    SubGhzTxRx* instance,
    const char* preset_name,
    uint32_t frequency,
    uint8_t* preset_data,
    size_t preset_data_size) {
    furi_assert(instance);
    furi_string_set(instance->preset->name, preset_name);
    SubGhzRadioPreset* preset = instance->preset;
    preset->frequency = frequency;
    preset->data = preset_data;
    preset->data_size = preset_data_size;
}

const char* subghz_txrx_get_preset_name(SubGhzTxRx* instance, const char* preset) {
    UNUSED(instance);
    const char* preset_name = "";
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

void subghz_txrx_get_frequency_and_modulation(
    SubGhzTxRx* instance,
    FuriString* frequency,
    FuriString* modulation) {
    furi_assert(instance);
    SubGhzRadioPreset* preset = instance->preset;
    if(frequency != NULL) {
        furi_string_printf(
            frequency,
            "%03ld.%02ld",
            preset->frequency / 1000000 % 1000,
            preset->frequency / 10000 % 100);
    }
    if(modulation != NULL) {
        furi_string_printf(modulation, "%.2s", furi_string_get_cstr(preset->name));
    }
}

static void subghz_txrx_begin(SubGhzTxRx* instance, uint8_t* preset_data) {
    furi_assert(instance);
    subghz_devices_reset(instance->radio_device);
    subghz_devices_idle(instance->radio_device);
    subghz_devices_load_preset(instance->radio_device, FuriHalSubGhzPresetCustom, preset_data);
    instance->txrx_state = SubGhzTxRxStateIDLE;
}

static uint32_t subghz_txrx_rx(SubGhzTxRx* instance, uint32_t frequency) {
    furi_assert(instance);

    furi_assert(
        instance->txrx_state != SubGhzTxRxStateRx && instance->txrx_state != SubGhzTxRxStateSleep);

    subghz_devices_idle(instance->radio_device);

    uint32_t value = subghz_devices_set_frequency(instance->radio_device, frequency);
    subghz_devices_flush_rx(instance->radio_device);
    subghz_txrx_speaker_on(instance);

    subghz_devices_start_async_rx(
        instance->radio_device, subghz_worker_rx_callback, instance->worker);
    subghz_worker_start(instance->worker);
    instance->txrx_state = SubGhzTxRxStateRx;
    return value;
}

static void subghz_txrx_idle(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->txrx_state != SubGhzTxRxStateSleep) {
        subghz_devices_idle(instance->radio_device);
        subghz_txrx_speaker_off(instance);
        instance->txrx_state = SubGhzTxRxStateIDLE;
    }
}

static void subghz_txrx_rx_end(SubGhzTxRx* instance) {
    furi_assert(instance);
    furi_assert(instance->txrx_state == SubGhzTxRxStateRx);

    if(subghz_worker_is_running(instance->worker)) {
        subghz_worker_stop(instance->worker);
        subghz_devices_stop_async_rx(instance->radio_device);
    }
    subghz_devices_idle(instance->radio_device);
    subghz_txrx_speaker_off(instance);
    instance->txrx_state = SubGhzTxRxStateIDLE;
}

void subghz_txrx_sleep(SubGhzTxRx* instance) {
    furi_assert(instance);
    subghz_devices_sleep(instance->radio_device);
    instance->txrx_state = SubGhzTxRxStateSleep;
}

static bool subghz_txrx_tx(SubGhzTxRx* instance, uint32_t frequency) {
    furi_assert(instance);
    furi_assert(instance->txrx_state != SubGhzTxRxStateSleep);
    subghz_devices_idle(instance->radio_device);
    subghz_devices_set_frequency(instance->radio_device, frequency);

    bool ret = subghz_devices_set_tx(instance->radio_device);
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
        ret = SubGhzTxRxStartTxStateOk;

        SubGhzRadioPreset* preset = instance->preset;
        instance->transmitter =
            subghz_transmitter_alloc_init(instance->environment, furi_string_get_cstr(temp_str));

        if(instance->transmitter) {
            if(subghz_transmitter_deserialize(instance->transmitter, flipper_format) ==
               SubGhzProtocolStatusOk) {
                if(strcmp(furi_string_get_cstr(preset->name), "") != 0) {
                    subghz_txrx_begin(
                        instance,
                        subghz_setting_get_preset_data_by_name(
                            instance->setting, furi_string_get_cstr(preset->name)));
                    if(preset->frequency) {
                        if(!subghz_txrx_tx(instance, preset->frequency)) {
                            FURI_LOG_E(TAG, "Only Rx");
                            ret = SubGhzTxRxStartTxStateErrorOnlyRx;
                        }
                    } else {
                        ret = SubGhzTxRxStartTxStateErrorParserOthers;
                    }

                } else {
                    FURI_LOG_E(
                        TAG, "Unknown name preset \" %s \"", furi_string_get_cstr(preset->name));
                    ret = SubGhzTxRxStartTxStateErrorParserOthers;
                }

                if(ret == SubGhzTxRxStartTxStateOk) {
                    //Start TX
                    subghz_devices_start_async_tx(
                        instance->radio_device, subghz_transmitter_yield, instance->transmitter);
                }
            } else {
                ret = SubGhzTxRxStartTxStateErrorParserOthers;
            }
        } else {
            ret = SubGhzTxRxStartTxStateErrorParserOthers;
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

void subghz_txrx_set_need_save_callback(
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
    subghz_devices_stop_async_tx(instance->radio_device);
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
}

FlipperFormat* subghz_txrx_get_fff_data(SubGhzTxRx* instance) {
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
        rssi = subghz_devices_get_rssi(instance->radio_device);

        // Stay if RSSI is high enough
        if(rssi > -90.0f) {
            instance->hopper_timeout = 10;
            instance->hopper_state = SubGhzHopperStateRSSITimeOut;
            return;
        }
    } else {
        instance->hopper_state = SubGhzHopperStateRunnig;
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

void subghz_txrx_hopper_unpause(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->hopper_state == SubGhzHopperStatePause) {
        instance->hopper_state = SubGhzHopperStateRunnig;
    }
}

void subghz_txrx_hopper_pause(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->hopper_state == SubGhzHopperStateRunnig) {
        instance->hopper_state = SubGhzHopperStatePause;
    }
}

void subghz_txrx_speaker_on(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->speaker_state == SubGhzSpeakerStateEnable) {
        if(furi_hal_speaker_acquire(30)) {
            subghz_devices_set_async_mirror_pin(instance->radio_device, &gpio_speaker);
        } else {
            instance->speaker_state = SubGhzSpeakerStateDisable;
        }
    }
}

void subghz_txrx_speaker_off(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->speaker_state != SubGhzSpeakerStateDisable) {
        if(furi_hal_speaker_is_mine()) {
            subghz_devices_set_async_mirror_pin(instance->radio_device, NULL);
            furi_hal_speaker_release();
            if(instance->speaker_state == SubGhzSpeakerStateShutdown)
                instance->speaker_state = SubGhzSpeakerStateDisable;
        }
    }
}

void subghz_txrx_speaker_mute(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->speaker_state == SubGhzSpeakerStateEnable) {
        if(furi_hal_speaker_is_mine()) {
            subghz_devices_set_async_mirror_pin(instance->radio_device, NULL);
        }
    }
}

void subghz_txrx_speaker_unmute(SubGhzTxRx* instance) {
    furi_assert(instance);
    if(instance->speaker_state == SubGhzSpeakerStateEnable) {
        if(furi_hal_speaker_is_mine()) {
            subghz_devices_set_async_mirror_pin(instance->radio_device, &gpio_speaker);
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

bool subghz_txrx_protocol_is_serializable(SubGhzTxRx* instance) {
    furi_assert(instance);
    return (instance->decoder_result->protocol->flag & SubGhzProtocolFlag_Save) ==
           SubGhzProtocolFlag_Save;
}

bool subghz_txrx_protocol_is_transmittable(SubGhzTxRx* instance, bool check_type) {
    furi_assert(instance);
    const SubGhzProtocol* protocol = instance->decoder_result->protocol;
    if(check_type) {
        return ((protocol->flag & SubGhzProtocolFlag_Send) == SubGhzProtocolFlag_Send) &&
               protocol->encoder->deserialize && protocol->type == SubGhzProtocolTypeStatic;
    }
    return ((protocol->flag & SubGhzProtocolFlag_Send) == SubGhzProtocolFlag_Send) &&
           protocol->encoder->deserialize;
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

void subghz_txrx_set_raw_file_encoder_worker_callback_end(
    SubGhzTxRx* instance,
    SubGhzProtocolEncoderRAWCallbackEnd callback,
    void* context) {
    subghz_protocol_raw_file_encoder_worker_set_callback_end(
        (SubGhzProtocolEncoderRAW*)subghz_transmitter_get_protocol_instance(instance->transmitter),
        callback,
        context);
}

bool subghz_txrx_radio_device_is_external_connected(SubGhzTxRx* instance, const char* name) {
    furi_assert(instance);

    bool is_connect = false;
    bool is_otg_enabled = furi_hal_power_is_otg_enabled();

    if(!is_otg_enabled) {
        subghz_txrx_radio_device_power_on(instance);
    }

    const SubGhzDevice* device = subghz_devices_get_by_name(name);
    if(device) {
        is_connect = subghz_devices_is_connect(device);
    }

    if(!is_otg_enabled) {
        subghz_txrx_radio_device_power_off(instance);
    }
    return is_connect;
}

SubGhzRadioDeviceType
    subghz_txrx_radio_device_set(SubGhzTxRx* instance, SubGhzRadioDeviceType radio_device_type) {
    furi_assert(instance);

    if(radio_device_type == SubGhzRadioDeviceTypeExternalCC1101 &&
       subghz_txrx_radio_device_is_external_connected(instance, SUBGHZ_DEVICE_CC1101_EXT_NAME)) {
        subghz_txrx_radio_device_power_on(instance);
        instance->radio_device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_EXT_NAME);
        subghz_devices_begin(instance->radio_device);
        instance->radio_device_type = SubGhzRadioDeviceTypeExternalCC1101;
    } else {
        subghz_txrx_radio_device_power_off(instance);
        if(instance->radio_device_type != SubGhzRadioDeviceTypeInternal) {
            subghz_devices_end(instance->radio_device);
        }
        instance->radio_device = subghz_devices_get_by_name(SUBGHZ_DEVICE_CC1101_INT_NAME);
        instance->radio_device_type = SubGhzRadioDeviceTypeInternal;
    }

    return instance->radio_device_type;
}

SubGhzRadioDeviceType subghz_txrx_radio_device_get(SubGhzTxRx* instance) {
    furi_assert(instance);
    return instance->radio_device_type;
}

float subghz_txrx_radio_device_get_rssi(SubGhzTxRx* instance) {
    furi_assert(instance);
    return subghz_devices_get_rssi(instance->radio_device);
}

const char* subghz_txrx_radio_device_get_name(SubGhzTxRx* instance) {
    furi_assert(instance);
    return subghz_devices_get_name(instance->radio_device);
}

bool subghz_txrx_radio_device_is_frequecy_valid(SubGhzTxRx* instance, uint32_t frequency) {
    furi_assert(instance);
    return subghz_devices_is_frequency_valid(instance->radio_device, frequency);
}
