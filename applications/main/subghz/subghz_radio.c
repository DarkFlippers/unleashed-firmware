#include "subghz_radio.h"
#include <lib/subghz/protocols/protocol_items.h>

#define TAG "SubGhz"

SubGhzTxRx* subghz_txrx_alloc() {
    SubGhzTxRx* txrx = malloc(sizeof(SubGhzTxRx));
    txrx->setting = subghz_setting_alloc();
    subghz_setting_load(txrx->setting, EXT_PATH("subghz/assets/setting_user"));

    txrx->preset = malloc(sizeof(SubGhzRadioPreset));
    txrx->preset->name = furi_string_alloc();
    subghz_set_preset(txrx, "AM650", subghz_setting_get_default_frequency(txrx->setting), NULL, 0);

    txrx->txrx_state = SubGhzTxRxStateSleep;

    subghz_hopper_set_state(txrx, SubGhzHopperStateOFF);
    subghz_speaker_set_state(txrx, SubGhzSpeakerStateDisable);
    subghz_txrx_set_debug_pin_state(txrx, false);

    txrx->worker = subghz_worker_alloc();
    txrx->fff_data = flipper_format_string_alloc();

    txrx->environment = subghz_environment_alloc();
    txrx->load_database = subghz_environment_load_keystore(
        txrx->environment, EXT_PATH("subghz/assets/keeloq_mfcodes"));
    subghz_environment_load_keystore(
        txrx->environment, EXT_PATH("subghz/assets/keeloq_mfcodes_user"));
    subghz_environment_set_came_atomo_rainbow_table_file_name(
        txrx->environment, EXT_PATH("subghz/assets/came_atomo"));
    subghz_environment_set_alutech_at_4n_rainbow_table_file_name(
        txrx->environment, EXT_PATH("subghz/assets/alutech_at_4n"));
    subghz_environment_set_nice_flor_s_rainbow_table_file_name(
        txrx->environment, EXT_PATH("subghz/assets/nice_flor_s"));
    subghz_environment_set_protocol_registry(txrx->environment, (void*)&subghz_protocol_registry);
    txrx->receiver = subghz_receiver_alloc_init(txrx->environment);

    subghz_worker_set_overrun_callback(
        txrx->worker, (SubGhzWorkerOverrunCallback)subghz_receiver_reset);
    subghz_worker_set_pair_callback(
        txrx->worker, (SubGhzWorkerPairCallback)subghz_receiver_decode);
    subghz_worker_set_context(txrx->worker, txrx->receiver);

    return txrx;
}

void subghz_txrx_free(SubGhzTxRx* txrx) {
    furi_assert(txrx);

    subghz_worker_free(txrx->worker);
    subghz_receiver_free(txrx->receiver);
    subghz_environment_free(txrx->environment);
    flipper_format_free(txrx->fff_data);
    furi_string_free(txrx->preset->name);
    subghz_setting_free(txrx->setting);
    free(txrx->preset);
    free(txrx);
}

bool subghz_txrx_is_load_database(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    return txrx->load_database;
}

void subghz_txrx_set_debug_pin_state(SubGhzTxRx* txrx, bool state) {
    furi_assert(txrx);
    txrx->debug_pin_state = state;
}

bool subghz_txrx_get_debug_pin_state(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    return txrx->debug_pin_state;
}

void subghz_set_preset(
    SubGhzTxRx* txrx,
    const char* preset_name,
    uint32_t frequency,
    uint8_t* preset_data,
    size_t preset_data_size) {
    furi_assert(txrx);
    furi_string_set(txrx->preset->name, preset_name);
    txrx->preset->frequency = frequency;
    txrx->preset->data = preset_data;
    txrx->preset->data_size = preset_data_size;
}

void subghz_get_frequency_modulation(
    SubGhzTxRx* txrx,
    FuriString* frequency,
    FuriString* modulation,
    bool long_name) {
    furi_assert(txrx);
    if(frequency != NULL) {
        furi_string_printf(
            frequency,
            "%03ld.%02ld",
            txrx->preset->frequency / 1000000 % 1000,
            txrx->preset->frequency / 10000 % 100);
    }
    if(modulation != NULL) {
        if(long_name) {
            furi_string_printf(modulation, "%s", furi_string_get_cstr(txrx->preset->name));
        } else {
            furi_string_printf(modulation, "%.2s", furi_string_get_cstr(txrx->preset->name));
        }
    }
}

const char* subghz_get_name_preset(SubGhzTxRx* txrx, const char* preset) {
    UNUSED(txrx);
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

SubGhzRadioPreset subghz_get_preset(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    return *txrx->preset;
}

static void subghz_begin(SubGhzTxRx* txrx, uint8_t* preset_data) {
    furi_assert(txrx);
    furi_hal_subghz_reset();
    furi_hal_subghz_idle();
    furi_hal_subghz_load_custom_preset(preset_data);
    furi_hal_gpio_init(furi_hal_subghz.cc1101_g0_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);
    txrx->txrx_state = SubGhzTxRxStateIDLE;
}

static uint32_t subghz_rx(SubGhzTxRx* txrx, uint32_t frequency) {
    furi_assert(txrx);
    if(!furi_hal_subghz_is_frequency_valid(frequency)) {
        furi_crash("SubGhz: Incorrect RX frequency.");
    }
    furi_assert(txrx->txrx_state != SubGhzTxRxStateRx && txrx->txrx_state != SubGhzTxRxStateSleep);

    furi_hal_subghz_idle();
    uint32_t value = furi_hal_subghz_set_frequency_and_path(frequency);
    furi_hal_gpio_init(furi_hal_subghz.cc1101_g0_pin, GpioModeInput, GpioPullNo, GpioSpeedLow);
    furi_hal_subghz_flush_rx();
    subghz_speaker_on(txrx);
    furi_hal_subghz_rx();

    furi_hal_subghz_start_async_rx(subghz_worker_rx_callback, txrx->worker);
    subghz_worker_start(txrx->worker);
    txrx->txrx_state = SubGhzTxRxStateRx;
    return value;
}

static void subghz_idle(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    furi_assert(txrx->txrx_state != SubGhzTxRxStateSleep);
    furi_hal_subghz_idle();
    subghz_speaker_off(txrx);
    txrx->txrx_state = SubGhzTxRxStateIDLE;
}

static void subghz_rx_end(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    furi_assert(txrx->txrx_state == SubGhzTxRxStateRx);

    if(subghz_worker_is_running(txrx->worker)) {
        subghz_worker_stop(txrx->worker);
        furi_hal_subghz_stop_async_rx();
    }
    furi_hal_subghz_idle();
    subghz_speaker_off(txrx);
    txrx->txrx_state = SubGhzTxRxStateIDLE;
}

void subghz_sleep(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    furi_hal_subghz_sleep();
    txrx->txrx_state = SubGhzTxRxStateSleep;
}

static bool subghz_tx(SubGhzTxRx* txrx, uint32_t frequency) {
    furi_assert(txrx);
    if(!furi_hal_subghz_is_frequency_valid(frequency)) {
        furi_crash("SubGhz: Incorrect TX frequency.");
    }
    furi_assert(txrx->txrx_state != SubGhzTxRxStateSleep);
    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency_and_path(frequency);
    furi_hal_gpio_write(furi_hal_subghz.cc1101_g0_pin, false);
    furi_hal_gpio_init(
        furi_hal_subghz.cc1101_g0_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    bool ret = furi_hal_subghz_tx();
    if(ret) {
        subghz_speaker_on(txrx);
        txrx->txrx_state = SubGhzTxRxStateTx;
    }
    return ret;
}

bool subghz_tx_start(SubGhzTxRx* txrx, FlipperFormat* flipper_format) {
    furi_assert(txrx);
    furi_assert(flipper_format);

    subghz_txrx_stop(txrx);

    bool ret = false;
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

        txrx->transmitter =
            subghz_transmitter_alloc_init(txrx->environment, furi_string_get_cstr(temp_str));

        if(txrx->transmitter) {
            if(subghz_transmitter_deserialize(txrx->transmitter, flipper_format) ==
               SubGhzProtocolStatusOk) {
                if(strcmp(furi_string_get_cstr(txrx->preset->name), "") != 0) {
                    subghz_begin(
                        txrx,
                        subghz_setting_get_preset_data_by_name(
                            txrx->setting, furi_string_get_cstr(txrx->preset->name)));
                } else {
                    FURI_LOG_E(
                        TAG,
                        "Unknown name preset \" %s \"",
                        furi_string_get_cstr(txrx->preset->name));
                    subghz_begin(
                        txrx, subghz_setting_get_preset_data_by_name(txrx->setting, "AM650"));
                }
                if(txrx->preset->frequency) {
                    ret = subghz_tx(txrx, txrx->preset->frequency);
                } else {
                    ret = subghz_tx(txrx, 433920000);
                }
                if(ret) {
                    //Start TX
                    furi_hal_subghz_start_async_tx(subghz_transmitter_yield, txrx->transmitter);
                } else {
                    //Todo: Show error
                    //subghz_dialog_message_show_only_rx(subghz);
                }
            } else {
                //Todo: Show error
                // dialog_message_show_storage_error(
                //     dialogs, "Error in protocol\nparameters\ndescription");
            }
        }
        if(!ret) {
            subghz_transmitter_free(txrx->transmitter);
            if(txrx->txrx_state != SubGhzTxRxStateSleep) {
                subghz_idle(txrx);
            }
        }

    } while(false);
    furi_string_free(temp_str);
    return ret;
}

void subghz_rx_start(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    subghz_txrx_stop(txrx);
    subghz_begin(
        txrx,
        subghz_setting_get_preset_data_by_name(
            subghz_txrx_get_setting(txrx), furi_string_get_cstr(txrx->preset->name)));
    subghz_rx(txrx, txrx->preset->frequency);
}

void subghz_txrx_need_save_callback_set(
    SubGhzTxRx* txrx,
    SubGhzTxRxNeedSaveCallback callback,
    void* context) {
    furi_assert(txrx);
    txrx->need_save_callback = callback;
    txrx->need_save_context = context;
}

static void subghz_tx_stop(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    furi_assert(txrx->txrx_state == SubGhzTxRxStateTx);
    //Stop TX
    furi_hal_subghz_stop_async_tx();
    subghz_transmitter_stop(txrx->transmitter);
    subghz_transmitter_free(txrx->transmitter);

    //if protocol dynamic then we save the last upload
    if(txrx->decoder_result->protocol->type == SubGhzProtocolTypeDynamic) {
        if(txrx->need_save_callback) {
            txrx->need_save_callback(txrx->need_save_context);
        }
    }
    subghz_idle(txrx);
    subghz_speaker_off(txrx);
    //Todo: Show message
    // notification_message(notifications, &sequence_reset_red);
}

FlipperFormat* subghz_txtx_get_fff_data(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    return txrx->fff_data;
}

SubGhzSetting* subghz_txrx_get_setting(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    return txrx->setting;
}

void subghz_txrx_stop(SubGhzTxRx* txrx) {
    furi_assert(txrx);

    switch(txrx->txrx_state) {
    case SubGhzTxRxStateTx:
        subghz_tx_stop(txrx);
        subghz_speaker_unmute(txrx);
        //subghz_sleep(txrx);
        break;
    case SubGhzTxRxStateRx:
        subghz_rx_end(txrx);
        subghz_speaker_mute(txrx);
        //subghz_sleep(txrx);
        break;

    default:
        break;
    }
}

SubGhzTxRxState subghz_txrx_get_state(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    return txrx->txrx_state;
}

void subghz_hopper_update(SubGhzTxRx* txrx) {
    furi_assert(txrx);

    switch(txrx->hopper_state) {
    case SubGhzHopperStateOFF:
    case SubGhzHopperStatePause:
        return;
    case SubGhzHopperStateRSSITimeOut:
        if(txrx->hopper_timeout != 0) {
            txrx->hopper_timeout--;
            return;
        }
        break;
    default:
        break;
    }
    float rssi = -127.0f;
    if(txrx->hopper_state != SubGhzHopperStateRSSITimeOut) {
        // See RSSI Calculation timings in CC1101 17.3 RSSI
        rssi = furi_hal_subghz_get_rssi();

        // Stay if RSSI is high enough
        if(rssi > -90.0f) {
            txrx->hopper_timeout = 10;
            txrx->hopper_state = SubGhzHopperStateRSSITimeOut;
            return;
        }
    } else {
        txrx->hopper_state = SubGhzHopperStateRunning;
    }
    // Select next frequency
    if(txrx->hopper_idx_frequency < subghz_setting_get_hopper_frequency_count(txrx->setting) - 1) {
        txrx->hopper_idx_frequency++;
    } else {
        txrx->hopper_idx_frequency = 0;
    }

    if(txrx->txrx_state == SubGhzTxRxStateRx) {
        subghz_rx_end(txrx);
    };
    if(txrx->txrx_state == SubGhzTxRxStateIDLE) {
        subghz_receiver_reset(txrx->receiver);
        txrx->preset->frequency =
            subghz_setting_get_hopper_frequency(txrx->setting, txrx->hopper_idx_frequency);
        subghz_rx(txrx, txrx->preset->frequency);
    }
}

SubGhzHopperState subghz_hopper_get_state(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    return txrx->hopper_state;
}

void subghz_hopper_set_state(SubGhzTxRx* txrx, SubGhzHopperState state) {
    furi_assert(txrx);
    txrx->hopper_state = state;
}

void subghz_hopper_remove_pause(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    if(txrx->hopper_state == SubGhzHopperStatePause) {
        txrx->hopper_state = SubGhzHopperStateRunning;
    }
}

void subghz_subghz_hopper_set_pause(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    if(txrx->hopper_state == SubGhzHopperStateRunning) {
        txrx->hopper_state = SubGhzHopperStatePause;
    }
}

void subghz_speaker_on(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    if(txrx->debug_pin_state) {
        furi_hal_subghz_set_async_mirror_pin(&gpio_ibutton);
    }

    if(txrx->speaker_state == SubGhzSpeakerStateEnable) {
        if(furi_hal_speaker_acquire(30)) {
            if(!txrx->debug_pin_state) {
                furi_hal_subghz_set_async_mirror_pin(&gpio_speaker);
            }
        } else {
            txrx->speaker_state = SubGhzSpeakerStateDisable;
        }
    }
}

void subghz_speaker_off(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    if(txrx->debug_pin_state) {
        furi_hal_subghz_set_async_mirror_pin(NULL);
    }
    if(txrx->speaker_state != SubGhzSpeakerStateDisable) {
        if(furi_hal_speaker_is_mine()) {
            if(!txrx->debug_pin_state) {
                furi_hal_subghz_set_async_mirror_pin(NULL);
            }
            furi_hal_speaker_release();
            if(txrx->speaker_state == SubGhzSpeakerStateShutdown)
                txrx->speaker_state = SubGhzSpeakerStateDisable;
        }
    }
}

void subghz_speaker_mute(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    if(txrx->debug_pin_state) {
        furi_hal_subghz_set_async_mirror_pin(NULL);
    }
    if(txrx->speaker_state == SubGhzSpeakerStateEnable) {
        if(furi_hal_speaker_is_mine()) {
            if(!txrx->debug_pin_state) {
                furi_hal_subghz_set_async_mirror_pin(NULL);
            }
        }
    }
}

void subghz_speaker_unmute(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    if(txrx->debug_pin_state) {
        furi_hal_subghz_set_async_mirror_pin(&gpio_ibutton);
    }
    if(txrx->speaker_state == SubGhzSpeakerStateEnable) {
        if(furi_hal_speaker_is_mine()) {
            if(!txrx->debug_pin_state) {
                furi_hal_subghz_set_async_mirror_pin(&gpio_speaker);
            }
        }
    }
}

void subghz_speaker_set_state(SubGhzTxRx* txrx, SubGhzSpeakerState state) {
    furi_assert(txrx);
    txrx->speaker_state = state;
}

SubGhzSpeakerState subghz_speaker_get_state(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    return txrx->speaker_state;
}

bool subghz_txrx_load_decoder_by_name_protocol(SubGhzTxRx* txrx, const char* name_protocol) {
    furi_assert(txrx);
    furi_assert(name_protocol);
    bool res = false;
    txrx->decoder_result = NULL;
    txrx->decoder_result =
        subghz_receiver_search_decoder_base_by_name(txrx->receiver, name_protocol);
    if(txrx->decoder_result) {
        res = true;
    }
    return res;
}

SubGhzProtocolDecoderBase* subghz_txrx_get_decoder(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    return txrx->decoder_result;
}

bool subghz_txrx_protocol_is_preserved(SubGhzTxRx* txrx) {
    furi_assert(txrx);
    return (
        (txrx->decoder_result->protocol->flag & SubGhzProtocolFlag_Save) ==
        SubGhzProtocolFlag_Save);
}

bool subghz_txrx_protocol_is_send(SubGhzTxRx* txrx, bool check_type) {
    furi_assert(txrx);
    if(check_type) {
        return (
            ((txrx->decoder_result->protocol->flag & SubGhzProtocolFlag_Send) ==
             SubGhzProtocolFlag_Send) &&
            txrx->decoder_result->protocol->encoder->deserialize &&
            txrx->decoder_result->protocol->type == SubGhzProtocolTypeStatic);
    }
    return (
        ((txrx->decoder_result->protocol->flag & SubGhzProtocolFlag_Send) ==
         SubGhzProtocolFlag_Send) &&
        txrx->decoder_result->protocol->encoder->deserialize);
}

void subghz_txrx_receiver_set_filter(SubGhzTxRx* txrx, SubGhzProtocolFlag filter) {
    furi_assert(txrx);
    subghz_receiver_set_filter(txrx->receiver, filter);
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

bool subghz_gen_data_protocol(
    void* context,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit) {
    furi_assert(context);
    SubGhzTxRx* txrx = context;

    bool res = false;

    subghz_set_preset(txrx, preset_name, frequency, NULL, 0);
    txrx->decoder_result =
        subghz_receiver_search_decoder_base_by_name(txrx->receiver, protocol_name);

    if(txrx->decoder_result == NULL) {
        //TODO: Error
        // furi_string_set(error_str, "Protocol not\nfound!");
        // scene_manager_next_scene(scene_manager, SubGhzSceneShowErrorSub);
        return false;
    }

    do {
        Stream* fff_data_stream = flipper_format_get_raw_stream(txrx->fff_data);
        stream_clean(fff_data_stream);
        if(subghz_protocol_decoder_base_serialize(
               txrx->decoder_result, txrx->fff_data, txrx->preset) != SubGhzProtocolStatusOk) {
            FURI_LOG_E(TAG, "Unable to serialize");
            break;
        }
        if(!flipper_format_update_uint32(txrx->fff_data, "Bit", &bit, 1)) {
            FURI_LOG_E(TAG, "Unable to update Bit");
            break;
        }

        uint8_t key_data[sizeof(uint64_t)] = {0};
        for(size_t i = 0; i < sizeof(uint64_t); i++) {
            key_data[sizeof(uint64_t) - i - 1] = (key >> (i * 8)) & 0xFF;
        }
        if(!flipper_format_update_hex(txrx->fff_data, "Key", key_data, sizeof(uint64_t))) {
            FURI_LOG_E(TAG, "Unable to update Key");
            break;
        }
        res = true;
    } while(false);
    return res;
}

bool subghz_gen_data_protocol_and_te(
    SubGhzTxRx* txrx,
    const char* preset_name,
    uint32_t frequency,
    const char* protocol_name,
    uint64_t key,
    uint32_t bit,
    uint32_t te) {
    furi_assert(txrx);
    bool ret = false;
    if(subghz_gen_data_protocol(txrx, preset_name, frequency, protocol_name, key, bit)) {
        if(!flipper_format_update_uint32(txrx->fff_data, "TE", (uint32_t*)&te, 1)) {
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
    subghz_set_preset(txrx, preset_name, frequency, NULL, 0);

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
    subghz_set_preset(txrx, preset_name, frequency, NULL, 0);

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
    subghz_set_preset(txrx, preset_name, frequency, NULL, 0);

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
    subghz_set_preset(txrx, preset_name, frequency, NULL, 0);

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
    subghz_set_preset(txrx, preset_name, frequency, NULL, 0);

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
    subghz_set_preset(txrx, preset_name, frequency, NULL, 0);

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

bool subghz_gen_secplus_v2_protocol(
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
    subghz_set_preset(txrx, name_preset, frequency, NULL, 0);
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

bool subghz_gen_secplus_v1_protocol(SubGhzTxRx* txrx, const char* name_preset, uint32_t frequency) {
    furi_assert(txrx);

    bool ret = false;
    uint32_t serial = (uint32_t)rand();
    while(!subghz_protocol_secplus_v1_check_fixed(serial)) {
        serial = (uint32_t)rand();
    }
    if(subghz_gen_data_protocol(
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