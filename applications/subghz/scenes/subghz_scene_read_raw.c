#include "../subghz_i.h"
#include "../views/subghz_read_raw.h"
#include <lib/subghz/protocols/subghz_protocol_raw.h>
#include <lib/subghz/subghz_parser.h>

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

void subghz_scene_read_raw_callback(SubghzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

void subghz_scene_read_raw_on_enter(void* context) {
    SubGhz* subghz = context;

    if(subghz->txrx->rx_key_state == SubGhzRxKeyStateNeedSave) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubghzCustomEventViewReadRAWMore);
    } else {
        subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
    }

    subghz_scene_read_raw_update_statusbar(subghz);
    subghz_read_raw_set_callback(subghz->subghz_read_raw, subghz_scene_read_raw_callback, subghz);

    subghz->txrx->protocol_result = subghz_parser_get_by_name(subghz->txrx->parser, "RAW");
    furi_assert(subghz->txrx->protocol_result);

    subghz_worker_set_pair_callback(
        subghz->txrx->worker, (SubGhzWorkerPairCallback)subghz_parser_raw_parse);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewReadRAW);
}

bool subghz_scene_read_raw_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SubghzCustomEventViewReadRAWBack:
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            };
            subghz->txrx->frequency = subghz_frequencies[subghz_frequencies_433_92];
            subghz->txrx->preset = FuriHalSubGhzPresetOok650Async;
            subghz_protocol_raw_save_to_file_stop(
                (SubGhzProtocolRAW*)subghz->txrx->protocol_result);
            subghz->state_notifications = SubGhzNotificationStateIDLE;

            if(subghz->txrx->rx_key_state == SubGhzRxKeyStateAddKey) {
                subghz->txrx->rx_key_state = SubGhzRxKeyStateExit;
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneNeedSaving);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    subghz->scene_manager, SubGhzSceneStart);
            }

            return true;
            break;
        case SubghzCustomEventViewReadRAWConfig:
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzSceneReadRAW, SubghzCustomEventManagerSet);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiverConfig);
            return true;
            break;
        case SubghzCustomEventViewReadRAWIDLE:
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            };
            subghz_protocol_raw_save_to_file_stop(
                (SubGhzProtocolRAW*)subghz->txrx->protocol_result);
            subghz->state_notifications = SubGhzNotificationStateIDLE;

            subghz->txrx->rx_key_state = SubGhzRxKeyStateAddKey;

            return true;
            break;
        case SubghzCustomEventViewReadRAWREC:

            if(subghz->txrx->rx_key_state != SubGhzRxKeyStateIDLE) {
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneNeedSaving);
            } else {
                subghz_get_preset_name(subghz, subghz->error_str);
                if(subghz_protocol_raw_save_to_file_init(
                       (SubGhzProtocolRAW*)subghz->txrx->protocol_result,
                       "Raw_temp",
                       subghz->txrx->frequency,
                       string_get_cstr(subghz->error_str))) {
                    if((subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) ||
                       (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
                        subghz_begin(subghz, subghz->txrx->preset);
                        subghz_rx(subghz, subghz->txrx->frequency);
                    }
                    subghz->state_notifications = SubGhzNotificationStateRX;
                } else {
                    string_set(subghz->error_str, "No SD card");
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
                }
            }

            return true;
            break;
        case SubghzCustomEventViewReadRAWMore:
            if(strcmp(
                   subghz_protocol_raw_get_last_file_name(
                       (SubGhzProtocolRAW*)subghz->txrx->protocol_result),
                   "")) {
                strlcpy(
                    subghz->file_name,
                    subghz_protocol_raw_get_last_file_name(
                        (SubGhzProtocolRAW*)subghz->txrx->protocol_result),
                    strlen(subghz_protocol_raw_get_last_file_name(
                        (SubGhzProtocolRAW*)subghz->txrx->protocol_result)) +
                        1);
                //set the path to read the file
                string_t temp_str;
                string_init_printf(
                    temp_str,
                    "%s/%s%s",
                    SUBGHZ_APP_PATH_FOLDER,
                    subghz->file_name,
                    SUBGHZ_APP_EXTENSION);
                subghz_protocol_raw_set_last_file_name(
                    (SubGhzProtocolRAW*)subghz->txrx->protocol_result, string_get_cstr(temp_str));
                string_clear(temp_str);

                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReadRAWMenu);
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
                    (SubGhzProtocolRAW*)subghz->txrx->protocol_result));
            subghz_read_raw_add_data_rssi(subghz->subghz_read_raw, furi_hal_subghz_get_rssi());
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

    //Сallback restoration
    subghz_worker_set_pair_callback(
        subghz->txrx->worker, (SubGhzWorkerPairCallback)subghz_parser_parse);
}
