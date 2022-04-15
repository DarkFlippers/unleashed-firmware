#include "../subghz_i.h"
#include "../views/subghz_read_raw.h"
#include <dolphin/dolphin.h>
#include <lib/subghz/protocols/raw.h>
#include <lib/toolbox/path.h>

#define RAW_FILE_NAME "Raw_signal_"
#define TAG "SubGhzSceneReadRAW"

bool subghz_scene_read_raw_update_filename(SubGhz* subghz) {
    bool ret = false;
    //set the path to read the file
    string_t temp_str;
    string_init(temp_str);
    do {
        if(!flipper_format_rewind(subghz->txrx->fff_data)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        if(!flipper_format_read_string(subghz->txrx->fff_data, "File_name", temp_str)) {
            FURI_LOG_E(TAG, "Missing File_name");
            break;
        }

        path_extract_filename_no_ext(string_get_cstr(temp_str), temp_str);
        strncpy(subghz->file_name, string_get_cstr(temp_str), SUBGHZ_MAX_LEN_NAME);

        ret = true;
    } while(false);

    string_clear(temp_str);
    return ret;
}

static void subghz_scene_read_raw_update_statusbar(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;

    string_t frequency_str;
    string_t modulation_str;

    string_init(frequency_str);
    string_init(modulation_str);

    subghz_get_frequency_modulation(subghz, frequency_str, modulation_str);
    subghz_read_raw_add_data_statusbar(
        subghz->subghz_read_raw, string_get_cstr(frequency_str), string_get_cstr(modulation_str));

    string_clear(frequency_str);
    string_clear(modulation_str);
}

void subghz_scene_read_raw_callback(SubGhzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

void subghz_scene_read_raw_callback_end_tx(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(
        subghz->view_dispatcher, SubGhzCustomEventViewReadRAWSendStop);
}

void subghz_scene_read_raw_on_enter(void* context) {
    SubGhz* subghz = context;

    switch(subghz->txrx->rx_key_state) {
    case SubGhzRxKeyStateBack:
        subghz_read_raw_set_status(subghz->subghz_read_raw, SubGhzReadRAWStatusIDLE, "");
        break;
    case SubGhzRxKeyStateRAWLoad:
        subghz_read_raw_set_status(
            subghz->subghz_read_raw, SubGhzReadRAWStatusLoadKeyTX, subghz->file_name);
        subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
        break;
    case SubGhzRxKeyStateRAWSave:
        subghz_read_raw_set_status(
            subghz->subghz_read_raw, SubGhzReadRAWStatusSaveKey, subghz->file_name);
        subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
        break;
    default:
        subghz_read_raw_set_status(subghz->subghz_read_raw, SubGhzReadRAWStatusStart, "");
        subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
        break;
    }

    subghz_scene_read_raw_update_statusbar(subghz);

    //set callback view raw
    subghz_read_raw_set_callback(subghz->subghz_read_raw, subghz_scene_read_raw_callback, subghz);

    subghz->txrx->decoder_result =
        subghz_receiver_search_decoder_base_by_name(subghz->txrx->receiver, "RAW");
    furi_assert(subghz->txrx->decoder_result);

    //set filter RAW feed
    subghz_receiver_set_filter(subghz->txrx->receiver, SubGhzProtocolFlag_RAW);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdReadRAW);
}

bool subghz_scene_read_raw_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SubGhzCustomEventViewReadRAWBack:
            //Stop TX
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
            }
            //Stop RX
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            };
            //Stop save file
            subghz_protocol_raw_save_to_file_stop(
                (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result);
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            //needed save?
            if((subghz->txrx->rx_key_state == SubGhzRxKeyStateAddKey) ||
               (subghz->txrx->rx_key_state == SubGhzRxKeyStateBack)) {
                subghz->txrx->rx_key_state = SubGhzRxKeyStateExit;
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneNeedSaving);
            } else {
                //Restore default setting
                subghz->txrx->frequency = subghz_setting_get_default_frequency(subghz->setting);
                subghz->txrx->preset = FuriHalSubGhzPresetOok650Async;
                if(!scene_manager_search_and_switch_to_previous_scene(
                       subghz->scene_manager, SubGhzSceneSaved)) {
                    if(!scene_manager_search_and_switch_to_previous_scene(
                           subghz->scene_manager, SubGhzSceneStart)) {
                        scene_manager_stop(subghz->scene_manager);
                        view_dispatcher_stop(subghz->view_dispatcher);
                    }
                }
            }
            return true;
            break;

        case SubGhzCustomEventViewReadRAWTXRXStop:
            //Stop TX
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
            }
            //Stop RX
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            };
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            return true;
            break;

        case SubGhzCustomEventViewReadRAWConfig:
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerSet);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiverConfig);
            return true;
            break;

        case SubGhzCustomEventViewReadRAWErase:
            subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
            return true;
            break;

        case SubGhzCustomEventViewReadRAWVibro:
            notification_message(subghz->notifications, &sequence_single_vibro);
            return true;
            break;

        case SubGhzCustomEventViewReadRAWMore:
            if(subghz_scene_read_raw_update_filename(subghz)) {
                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerSet);
                subghz->txrx->rx_key_state = SubGhzRxKeyStateRAWLoad;
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneMoreRAW);
                return true;
            } else {
                furi_crash("SugGhz: RAW file name update error.");
            }
            break;

        case SubGhzCustomEventViewReadRAWSendStart:

            if(subghz_scene_read_raw_update_filename(subghz)) {
                //start send
                subghz->state_notifications = SubGhzNotificationStateIDLE;
                if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                    subghz_rx_end(subghz);
                }
                if((subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) ||
                   (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
                    //ToDo FIX

                    if(!subghz_tx_start(subghz, subghz->txrx->fff_data)) {
                        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowOnlyRx);
                    } else {
                        DOLPHIN_DEED(DolphinDeedSubGhzSend);
                        // set callback end tx
                        subghz_protocol_raw_file_encoder_worker_set_callback_end(
                            (SubGhzProtocolEncoderRAW*)subghz->txrx->transmitter->protocol_instance,
                            subghz_scene_read_raw_callback_end_tx,
                            subghz);
                        subghz->state_notifications = SubGhzNotificationStateTX;
                    }
                }
            }
            return true;
            break;

        case SubGhzCustomEventViewReadRAWSendStop:
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
            }
            subghz_read_raw_stop_send(subghz->subghz_read_raw);
            return true;
            break;

        case SubGhzCustomEventViewReadRAWIDLE:
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            };
            subghz_protocol_raw_save_to_file_stop(
                (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result);
            subghz_protocol_raw_gen_fff_data(subghz->txrx->fff_data, RAW_FILE_NAME);
            subghz->state_notifications = SubGhzNotificationStateIDLE;

            subghz->txrx->rx_key_state = SubGhzRxKeyStateAddKey;

            return true;
            break;

        case SubGhzCustomEventViewReadRAWREC:
            if(subghz->txrx->rx_key_state != SubGhzRxKeyStateIDLE) {
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneNeedSaving);
            } else {
                //subghz_get_preset_name(subghz, subghz->error_str);
                if(subghz_protocol_raw_save_to_file_init(
                       (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result,
                       RAW_FILE_NAME,
                       subghz->txrx->frequency,
                       subghz->txrx->preset)) {
                    DOLPHIN_DEED(DolphinDeedSubGhzRawRec);
                    if((subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) ||
                       (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
                        subghz_begin(subghz, subghz->txrx->preset);
                        subghz_rx(subghz, subghz->txrx->frequency);
                    }
                    subghz->state_notifications = SubGhzNotificationStateRX;
                    subghz->txrx->rx_key_state = SubGhzRxKeyStateAddKey;
                } else {
                    string_set(subghz->error_str, "Function requires\nan SD card.");
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
                }
            }
            return true;
            break;

        case SubGhzCustomEventViewReadRAWSave:
            if(subghz_scene_read_raw_update_filename(subghz)) {
                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerSetRAW);
                subghz->txrx->rx_key_state = SubGhzRxKeyStateBack;
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            }
            return true;
            break;

        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        switch(subghz->state_notifications) {
        case SubGhzNotificationStateRX:
            notification_message(subghz->notifications, &sequence_blink_blue_10);
            subghz_read_raw_update_sample_write(
                subghz->subghz_read_raw,
                subghz_protocol_raw_get_sample_write(
                    (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result));
            subghz_read_raw_add_data_rssi(subghz->subghz_read_raw, furi_hal_subghz_get_rssi());
            break;
        case SubGhzNotificationStateTX:
            notification_message(subghz->notifications, &sequence_blink_green_10);
            subghz_read_raw_update_sin(subghz->subghz_read_raw);
            break;
        default:
            break;
        }
    }
    return false;
}

void subghz_scene_read_raw_on_exit(void* context) {
    SubGhz* subghz = context;

    //Stop CC1101
    if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
        subghz_rx_end(subghz);
        subghz_sleep(subghz);
    };
    subghz->state_notifications = SubGhzNotificationStateIDLE;

    //filter restoration
    subghz_receiver_set_filter(subghz->txrx->receiver, SubGhzProtocolFlag_Decodable);
}
