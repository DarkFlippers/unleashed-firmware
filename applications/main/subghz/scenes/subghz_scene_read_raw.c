#include "../subghz_i.h"
#include "../views/subghz_read_raw.h"
#include <dolphin/dolphin.h>
#include <lib/subghz/protocols/raw.h>
#include <lib/toolbox/path.h>
#include <float_tools.h>

#define RAW_FILE_NAME "Raw_signal_"
#define TAG "SubGhzSceneReadRAW"
#define RAW_THRESHOLD_RSSI_LOW_COUNT 10

bool subghz_scene_read_raw_update_filename(SubGhz* subghz) {
    bool ret = false;
    //set the path to read the file
    FuriString* temp_str;
    temp_str = furi_string_alloc();
    do {
        if(!flipper_format_rewind(subghz->txrx->fff_data)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        if(!flipper_format_read_string(subghz->txrx->fff_data, "File_name", temp_str)) {
            FURI_LOG_E(TAG, "Missing File_name");
            break;
        }

        furi_string_set(subghz->file_path, temp_str);

        ret = true;
    } while(false);

    furi_string_free(temp_str);
    return ret;
}

static void subghz_scene_read_raw_update_statusbar(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;

    FuriString* frequency_str;
    FuriString* modulation_str;

    frequency_str = furi_string_alloc();
    modulation_str = furi_string_alloc();

    subghz_get_frequency_modulation(subghz, frequency_str, modulation_str);
    subghz_read_raw_add_data_statusbar(
        subghz->subghz_read_raw,
        furi_string_get_cstr(frequency_str),
        furi_string_get_cstr(modulation_str));

    furi_string_free(frequency_str);
    furi_string_free(modulation_str);
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
    FuriString* file_name;
    file_name = furi_string_alloc();

    switch(subghz->txrx->rx_key_state) {
    case SubGhzRxKeyStateBack:
        subghz_read_raw_set_status(
            subghz->subghz_read_raw, SubGhzReadRAWStatusIDLE, "", subghz->txrx->raw_threshold_rssi);
        break;
    case SubGhzRxKeyStateRAWLoad:
        path_extract_filename(subghz->file_path, file_name, true);
        subghz_read_raw_set_status(
            subghz->subghz_read_raw,
            SubGhzReadRAWStatusLoadKeyTX,
            furi_string_get_cstr(file_name),
            subghz->txrx->raw_threshold_rssi);
        subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
        break;
    case SubGhzRxKeyStateRAWSave:
        path_extract_filename(subghz->file_path, file_name, true);
        subghz_read_raw_set_status(
            subghz->subghz_read_raw,
            SubGhzReadRAWStatusSaveKey,
            furi_string_get_cstr(file_name),
            subghz->txrx->raw_threshold_rssi);
        subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
        break;
    default:
        subghz_read_raw_set_status(
            subghz->subghz_read_raw,
            SubGhzReadRAWStatusStart,
            "",
            subghz->txrx->raw_threshold_rssi);
        subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
        break;
    }
    furi_string_free(file_name);
    subghz_scene_read_raw_update_statusbar(subghz);

    //set callback view raw
    subghz_read_raw_set_callback(subghz->subghz_read_raw, subghz_scene_read_raw_callback, subghz);

    subghz->txrx->decoder_result = subghz_receiver_search_decoder_base_by_name(
        subghz->txrx->receiver, SUBGHZ_PROTOCOL_RAW_NAME);
    furi_assert(subghz->txrx->decoder_result);

    //set filter RAW feed
    subghz_receiver_set_filter(subghz->txrx->receiver, SubGhzProtocolFlag_RAW);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdReadRAW);
}

bool subghz_scene_read_raw_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;
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
                subghz_preset_init(
                    subghz,
                    "AM650",
                    subghz_setting_get_default_frequency(subghz->setting),
                    NULL,
                    0);
                if(!scene_manager_search_and_switch_to_previous_scene(
                       subghz->scene_manager, SubGhzSceneSaved)) {
                    if(!scene_manager_search_and_switch_to_previous_scene(
                           subghz->scene_manager, SubGhzSceneStart)) {
                        scene_manager_stop(subghz->scene_manager);
                        view_dispatcher_stop(subghz->view_dispatcher);
                    }
                }
            }
            consumed = true;
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
            consumed = true;
            break;

        case SubGhzCustomEventViewReadRAWConfig:
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerSet);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiverConfig);
            consumed = true;
            break;

        case SubGhzCustomEventViewReadRAWErase:
            if(subghz->txrx->rx_key_state == SubGhzRxKeyStateAddKey) {
                if(subghz_scene_read_raw_update_filename(subghz)) {
                    furi_string_set(subghz->file_path_tmp, subghz->file_path);
                    subghz_delete_file(subghz);
                }
            }
            subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
            notification_message(subghz->notifications, &sequence_reset_rgb);
            consumed = true;
            break;

        case SubGhzCustomEventViewReadRAWMore:
            if(subghz_file_available(subghz)) {
                if(subghz_scene_read_raw_update_filename(subghz)) {
                    scene_manager_set_scene_state(
                        subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerSet);
                    subghz->txrx->rx_key_state = SubGhzRxKeyStateRAWLoad;
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneMoreRAW);
                    consumed = true;
                } else {
                    furi_crash("SubGhz: RAW file name update error.");
                }
            } else {
                if(!scene_manager_search_and_switch_to_previous_scene(
                       subghz->scene_manager, SubGhzSceneStart)) {
                    scene_manager_stop(subghz->scene_manager);
                    view_dispatcher_stop(subghz->view_dispatcher);
                }
            }
            break;

        case SubGhzCustomEventViewReadRAWSendStart:

            if(subghz_file_available(subghz) && subghz_scene_read_raw_update_filename(subghz)) {
                //start send
                subghz->state_notifications = SubGhzNotificationStateIDLE;
                if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                    subghz_rx_end(subghz);
                }
                if((subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) ||
                   (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
                    if(!subghz_tx_start(subghz, subghz->txrx->fff_data)) {
                        subghz->txrx->rx_key_state = SubGhzRxKeyStateBack;
                        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowOnlyRx);
                    } else {
                        if(scene_manager_has_previous_scene(
                               subghz->scene_manager, SubGhzSceneSaved) ||
                           !scene_manager_has_previous_scene(
                               subghz->scene_manager, SubGhzSceneStart)) {
                            DOLPHIN_DEED(DolphinDeedSubGhzSend);
                        }
                        // set callback end tx
                        subghz_protocol_raw_file_encoder_worker_set_callback_end(
                            (SubGhzProtocolEncoderRAW*)subghz_transmitter_get_protocol_instance(
                                subghz->txrx->transmitter),
                            subghz_scene_read_raw_callback_end_tx,
                            subghz);
                        subghz->state_notifications = SubGhzNotificationStateTx;
                    }
                }
            } else {
                if(!scene_manager_search_and_switch_to_previous_scene(
                       subghz->scene_manager, SubGhzSceneStart)) {
                    scene_manager_stop(subghz->scene_manager);
                    view_dispatcher_stop(subghz->view_dispatcher);
                }
            }
            consumed = true;
            break;

        case SubGhzCustomEventViewReadRAWSendStop:
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_speaker_unmute(subghz);
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
            }
            subghz_read_raw_stop_send(subghz->subghz_read_raw);
            consumed = true;
            break;

        case SubGhzCustomEventViewReadRAWIDLE:
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            };

            size_t spl_count = subghz_protocol_raw_get_sample_write(
                (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result);

            subghz_protocol_raw_save_to_file_stop(
                (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result);

            FuriString* temp_str;
            temp_str = furi_string_alloc();
            furi_string_printf(
                temp_str, "%s/%s%s", SUBGHZ_RAW_FOLDER, RAW_FILE_NAME, SUBGHZ_APP_EXTENSION);
            subghz_protocol_raw_gen_fff_data(
                subghz->txrx->fff_data, furi_string_get_cstr(temp_str));
            furi_string_free(temp_str);

            if(spl_count > 0) {
                notification_message(subghz->notifications, &sequence_set_green_255);
            } else {
                notification_message(subghz->notifications, &sequence_reset_rgb);
            }

            subghz->state_notifications = SubGhzNotificationStateIDLE;
            subghz->txrx->rx_key_state = SubGhzRxKeyStateAddKey;

            consumed = true;
            break;

        case SubGhzCustomEventViewReadRAWREC:
            if(subghz->txrx->rx_key_state != SubGhzRxKeyStateIDLE) {
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneNeedSaving);
            } else {
                subghz->txrx->raw_threshold_rssi_low_count = RAW_THRESHOLD_RSSI_LOW_COUNT;
                if(subghz_protocol_raw_save_to_file_init(
                       (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result,
                       RAW_FILE_NAME,
                       subghz->txrx->preset)) {
                    DOLPHIN_DEED(DolphinDeedSubGhzRawRec);
                    if((subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) ||
                       (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
                        subghz_begin(
                            subghz,
                            subghz_setting_get_preset_data_by_name(
                                subghz->setting,
                                furi_string_get_cstr(subghz->txrx->preset->name)));
                        subghz_rx(subghz, subghz->txrx->preset->frequency);
                    }
                    subghz->state_notifications = SubGhzNotificationStateRx;
                    subghz->txrx->rx_key_state = SubGhzRxKeyStateAddKey;
                } else {
                    furi_string_set(subghz->error_str, "Function requires\nan SD card.");
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
                }
            }
            consumed = true;
            break;

        case SubGhzCustomEventViewReadRAWSave:
            if(subghz_file_available(subghz) && subghz_scene_read_raw_update_filename(subghz)) {
                scene_manager_set_scene_state(
                    subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerSetRAW);
                subghz->txrx->rx_key_state = SubGhzRxKeyStateBack;
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            } else {
                if(!scene_manager_search_and_switch_to_previous_scene(
                       subghz->scene_manager, SubGhzSceneStart)) {
                    scene_manager_stop(subghz->scene_manager);
                    view_dispatcher_stop(subghz->view_dispatcher);
                }
            }
            consumed = true;
            break;

        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        switch(subghz->state_notifications) {
        case SubGhzNotificationStateRx:
            notification_message(subghz->notifications, &sequence_blink_cyan_10);
            subghz_read_raw_update_sample_write(
                subghz->subghz_read_raw,
                subghz_protocol_raw_get_sample_write(
                    (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result));

            float rssi = furi_hal_subghz_get_rssi();

            if(float_is_equal(subghz->txrx->raw_threshold_rssi, SUBGHZ_RAW_TRESHOLD_MIN)) {
                subghz_read_raw_add_data_rssi(subghz->subghz_read_raw, rssi, true);
                subghz_protocol_raw_save_to_file_pause(
                    (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result, false);
            } else {
                if(rssi < subghz->txrx->raw_threshold_rssi) {
                    subghz->txrx->raw_threshold_rssi_low_count++;
                    if(subghz->txrx->raw_threshold_rssi_low_count > RAW_THRESHOLD_RSSI_LOW_COUNT) {
                        subghz->txrx->raw_threshold_rssi_low_count = RAW_THRESHOLD_RSSI_LOW_COUNT;
                    }
                    subghz_read_raw_add_data_rssi(subghz->subghz_read_raw, rssi, false);
                } else {
                    subghz->txrx->raw_threshold_rssi_low_count = 0;
                }

                if(subghz->txrx->raw_threshold_rssi_low_count == RAW_THRESHOLD_RSSI_LOW_COUNT) {
                    subghz_read_raw_add_data_rssi(subghz->subghz_read_raw, rssi, false);
                    subghz_protocol_raw_save_to_file_pause(
                        (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result, true);
                    subghz_speaker_mute(subghz);
                } else {
                    subghz_read_raw_add_data_rssi(subghz->subghz_read_raw, rssi, true);
                    subghz_protocol_raw_save_to_file_pause(
                        (SubGhzProtocolDecoderRAW*)subghz->txrx->decoder_result, false);
                    subghz_speaker_unmute(subghz);
                }
            }

            break;
        case SubGhzNotificationStateTx:
            notification_message(subghz->notifications, &sequence_blink_magenta_10);
            subghz_read_raw_update_sin(subghz->subghz_read_raw);
            break;
        default:
            break;
        }
    }
    return consumed;
}

void subghz_scene_read_raw_on_exit(void* context) {
    SubGhz* subghz = context;

    //Stop CC1101
    if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
        subghz_rx_end(subghz);
        subghz_sleep(subghz);
    };
    subghz->state_notifications = SubGhzNotificationStateIDLE;
    notification_message(subghz->notifications, &sequence_reset_rgb);

    //filter restoration
    subghz_receiver_set_filter(subghz->txrx->receiver, subghz->txrx->filter);
}
