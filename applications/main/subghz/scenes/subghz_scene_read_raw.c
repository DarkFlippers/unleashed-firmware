#include "../subghz_i.h"
#include "../views/subghz_read_raw.h"
#include <dolphin/dolphin.h>
#include <lib/subghz/protocols/raw.h>
#include <toolbox/path.h>

#define TAG "SubGhzSceneReadRaw"

#define RAW_FILE_NAME "Raw_signal_"

bool subghz_scene_read_raw_update_filename(SubGhz* subghz) {
    bool ret = false;
    //set the path to read the file
    FuriString* temp_str;
    temp_str = furi_string_alloc();
    do {
        FlipperFormat* fff_data = subghz_txrx_get_fff_data(subghz->txrx);
        if(!flipper_format_rewind(fff_data)) {
            FURI_LOG_E(TAG, "Rewind error");
            break;
        }

        if(!flipper_format_read_string(fff_data, "File_name", temp_str)) {
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

    FuriString* frequency_str = furi_string_alloc();
    FuriString* modulation_str = furi_string_alloc();

    subghz_txrx_get_frequency_and_modulation(subghz->txrx, frequency_str, modulation_str);
    subghz_read_raw_add_data_statusbar(
        subghz->subghz_read_raw,
        furi_string_get_cstr(frequency_str),
        furi_string_get_cstr(modulation_str));

    furi_string_free(frequency_str);
    furi_string_free(modulation_str);

    subghz_read_raw_set_radio_device_type(
        subghz->subghz_read_raw, subghz_txrx_radio_device_get(subghz->txrx));
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
    FuriString* file_name = furi_string_alloc();

    float threshold_rssi = subghz_threshold_rssi_get(subghz->threshold_rssi);
    switch(subghz_rx_key_state_get(subghz)) {
    case SubGhzRxKeyStateBack:
        subghz_read_raw_set_status(
            subghz->subghz_read_raw, SubGhzReadRAWStatusIDLE, "", threshold_rssi);
        break;
    case SubGhzRxKeyStateRAWLoad:
    case SubGhzRxKeyStateRAWMore:
        path_extract_filename(subghz->file_path, file_name, true);
        subghz_read_raw_set_status(
            subghz->subghz_read_raw,
            SubGhzReadRAWStatusLoadKeyTX,
            furi_string_get_cstr(file_name),
            threshold_rssi);
        break;
    case SubGhzRxKeyStateRAWSave:
        path_extract_filename(subghz->file_path, file_name, true);
        subghz_read_raw_set_status(
            subghz->subghz_read_raw,
            SubGhzReadRAWStatusSaveKey,
            furi_string_get_cstr(file_name),
            threshold_rssi);
        break;
    default:
        subghz_read_raw_set_status(
            subghz->subghz_read_raw, SubGhzReadRAWStatusStart, "", threshold_rssi);
        break;
    }

    if((subghz_rx_key_state_get(subghz) != SubGhzRxKeyStateBack) &&
       (subghz_rx_key_state_get(subghz) != SubGhzRxKeyStateRAWLoad)) {
        subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);
    }
    furi_string_free(file_name);
    subghz_scene_read_raw_update_statusbar(subghz);

    //set callback view raw
    subghz_read_raw_set_callback(subghz->subghz_read_raw, subghz_scene_read_raw_callback, subghz);

    furi_check(subghz_txrx_load_decoder_by_name_protocol(subghz->txrx, SUBGHZ_PROTOCOL_RAW_NAME));

    //set filter RAW feed
    subghz_txrx_receiver_set_filter(subghz->txrx, SubGhzProtocolFlag_RAW);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdReadRAW);
}

bool subghz_scene_read_raw_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;
    SubGhzProtocolDecoderRAW* decoder_raw =
        (SubGhzProtocolDecoderRAW*)subghz_txrx_get_decoder(subghz->txrx);
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SubGhzCustomEventViewReadRAWBack:

            subghz_txrx_stop(subghz->txrx);
            //Stop save file
            subghz_protocol_raw_save_to_file_stop(decoder_raw);
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            //needed save?
            if((subghz_rx_key_state_get(subghz) == SubGhzRxKeyStateAddKey) ||
               (subghz_rx_key_state_get(subghz) == SubGhzRxKeyStateBack)) {
                subghz_rx_key_state_set(subghz, SubGhzRxKeyStateExit);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneNeedSaving);
            } else {
                //Restore default setting
                subghz_set_default_preset(subghz);
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
            subghz_txrx_stop(subghz->txrx);
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
            if(subghz_rx_key_state_get(subghz) == SubGhzRxKeyStateAddKey) {
                if(subghz_scene_read_raw_update_filename(subghz)) {
                    furi_string_set(subghz->file_path_tmp, subghz->file_path);
                    subghz_delete_file(subghz);
                }
            }
            subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);
            notification_message(subghz->notifications, &sequence_reset_rgb);
            consumed = true;
            break;

        case SubGhzCustomEventViewReadRAWMore:
            if(subghz_file_available(subghz)) {
                if(subghz_scene_read_raw_update_filename(subghz)) {
                    scene_manager_set_scene_state(
                        subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerSet);
                    if(subghz_rx_key_state_get(subghz) != SubGhzRxKeyStateRAWLoad) {
                        subghz_rx_key_state_set(subghz, SubGhzRxKeyStateRAWMore);
                    }
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
                if(!subghz_tx_start(subghz, subghz_txrx_get_fff_data(subghz->txrx))) {
                    subghz_rx_key_state_set(subghz, SubGhzRxKeyStateBack);
                    subghz_read_raw_set_status(
                        subghz->subghz_read_raw,
                        SubGhzReadRAWStatusIDLE,
                        "",
                        subghz_threshold_rssi_get(subghz->threshold_rssi));
                } else {
                    if(scene_manager_has_previous_scene(subghz->scene_manager, SubGhzSceneSaved) ||
                       !scene_manager_has_previous_scene(subghz->scene_manager, SubGhzSceneStart)) {
                        dolphin_deed(DolphinDeedSubGhzSend);
                    }
                    // set callback end tx
                    subghz_txrx_set_raw_file_encoder_worker_callback_end(
                        subghz->txrx, subghz_scene_read_raw_callback_end_tx, subghz);
                    subghz->state_notifications = SubGhzNotificationStateTx;
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
            subghz_txrx_stop(subghz->txrx);
            subghz_read_raw_stop_send(subghz->subghz_read_raw);
            consumed = true;
            break;

        case SubGhzCustomEventViewReadRAWIDLE:
            subghz_txrx_stop(subghz->txrx);
            size_t spl_count = subghz_protocol_raw_get_sample_write(decoder_raw);

            subghz_protocol_raw_save_to_file_stop(decoder_raw);

            FuriString* temp_str = furi_string_alloc();
            furi_string_printf(
                temp_str,
                "%s/%s%s",
                SUBGHZ_RAW_FOLDER,
                RAW_FILE_NAME,
                SUBGHZ_APP_FILENAME_EXTENSION);
            subghz_protocol_raw_gen_fff_data(
                subghz_txrx_get_fff_data(subghz->txrx),
                furi_string_get_cstr(temp_str),
                subghz_txrx_radio_device_get_name(subghz->txrx));
            furi_string_free(temp_str);

            if(spl_count > 0) {
                notification_message(subghz->notifications, &sequence_set_green_255);
            } else {
                notification_message(subghz->notifications, &sequence_reset_rgb);
            }

            subghz->state_notifications = SubGhzNotificationStateIDLE;
            subghz_rx_key_state_set(subghz, SubGhzRxKeyStateAddKey);

            consumed = true;
            break;

        case SubGhzCustomEventViewReadRAWREC:
            if(subghz_rx_key_state_get(subghz) != SubGhzRxKeyStateIDLE) {
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneNeedSaving);
            } else {
                SubGhzRadioPreset preset = subghz_txrx_get_preset(subghz->txrx);
                if(subghz_protocol_raw_save_to_file_init(decoder_raw, RAW_FILE_NAME, &preset)) {
                    dolphin_deed(DolphinDeedSubGhzRawRec);
                    subghz_txrx_rx_start(subghz->txrx);
                    subghz->state_notifications = SubGhzNotificationStateRx;
                    subghz_rx_key_state_set(subghz, SubGhzRxKeyStateAddKey);
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
                subghz_rx_key_state_set(subghz, SubGhzRxKeyStateBack);
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
                subghz->subghz_read_raw, subghz_protocol_raw_get_sample_write(decoder_raw));

            SubGhzThresholdRssiData ret_rssi = subghz_threshold_get_rssi_data(
                subghz->threshold_rssi, subghz_txrx_radio_device_get_rssi(subghz->txrx));
            subghz_read_raw_add_data_rssi(
                subghz->subghz_read_raw, ret_rssi.rssi, ret_rssi.is_above);
            subghz_protocol_raw_save_to_file_pause(decoder_raw, !ret_rssi.is_above);
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
    subghz_txrx_stop(subghz->txrx);
    subghz->state_notifications = SubGhzNotificationStateIDLE;
    notification_message(subghz->notifications, &sequence_reset_rgb);

    //filter restoration
    subghz_txrx_receiver_set_filter(subghz->txrx, subghz->filter);
}
