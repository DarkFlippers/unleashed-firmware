#include "../subghz_i.h"
#include "../views/subghz_save_raw.h"
#include <lib/subghz/protocols/subghz_protocol_raw.h>
#include <lib/subghz/subghz_parser.h>

static void subghz_scene_save_raw_update_statusbar(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    char frequency_str[20];
    char preset_str[10];

    snprintf(
        frequency_str,
        sizeof(frequency_str),
        "%03ld.%02ld",
        subghz->txrx->frequency / 1000000 % 1000,
        subghz->txrx->frequency / 10000 % 100);
    if(subghz->txrx->preset == FuriHalSubGhzPresetOok650Async ||
       subghz->txrx->preset == FuriHalSubGhzPresetOok270Async) {
        snprintf(preset_str, sizeof(preset_str), "AM");
    } else if(
        subghz->txrx->preset == FuriHalSubGhzPreset2FSKDev238Async ||
        subghz->txrx->preset == FuriHalSubGhzPreset2FSKDev476Async) {
        snprintf(preset_str, sizeof(preset_str), "FM");
    } else {
        furi_crash(NULL);
    }
    subghz_save_raw_add_data_statusbar(subghz->subghz_save_raw, frequency_str, preset_str);
}

void subghz_scene_save_raw_callback(SubghzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

void subghz_scene_save_raw_on_enter(void* context) {
    SubGhz* subghz = context;
    subghz_scene_save_raw_update_statusbar(subghz);
    subghz_save_raw_set_callback(subghz->subghz_save_raw, subghz_scene_save_raw_callback, subghz);

    subghz->txrx->protocol_result = subghz_parser_get_by_name(subghz->txrx->parser, "RAW");
    furi_assert(subghz->txrx->protocol_result);

    subghz_worker_set_pair_callback(
        subghz->txrx->worker, (SubGhzWorkerPairCallback)subghz_parser_raw_parse);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewSaveRAW);
}

bool subghz_scene_save_raw_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SubghzCustomEventViewSaveRAWBack:
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            };
            subghz->txrx->frequency = subghz_frequencies[subghz_frequencies_433_92];
            subghz->txrx->preset = FuriHalSubGhzPresetOok650Async;
            subghz_protocol_save_raw_to_file_stop(
                (SubGhzProtocolRAW*)subghz->txrx->protocol_result);
            scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneStart);
            subghz->state_notifications = NOTIFICATION_IDLE_STATE;
            return true;
            break;
        case SubghzCustomEventViewSaveRAWConfig:
            scene_manager_set_scene_state(subghz->scene_manager, SubGhzSceneSaveRAW, 1);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiverConfig);
            return true;
            break;
        case SubghzCustomEventViewSaveRAWIDLE:
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            };
            subghz_protocol_save_raw_to_file_stop(
                (SubGhzProtocolRAW*)subghz->txrx->protocol_result);
            subghz->state_notifications = NOTIFICATION_IDLE_STATE;
            //send the name of the saved file to the account
            subghz_save_raw_set_file_name(
                subghz->subghz_save_raw,
                subghz_protocol_get_last_file_name(
                    (SubGhzProtocolRAW*)subghz->txrx->protocol_result));
            return true;
            break;
        case SubghzCustomEventViewSaveRAWREC:
            if(subghz_protocol_save_raw_to_file_init(
                   (SubGhzProtocolRAW*)subghz->txrx->protocol_result,
                   "Raw",
                   subghz->txrx->frequency,
                   subghz->txrx->preset)) {
                if((subghz->txrx->txrx_state == SubGhzTxRxStateIdle) ||
                   (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
                    subghz_begin(subghz, subghz->txrx->preset);
                    subghz_rx(subghz, subghz->txrx->frequency);
                }
                subghz->state_notifications = NOTIFICATION_RX_STATE;
            } else {
                string_set(subghz->error_str, "No SD card");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
            }
            return true;
            break;
        case SubghzCustomEventViewSaveRAWMore:
            if(strcmp(
                   subghz_protocol_get_last_file_name(
                       (SubGhzProtocolRAW*)subghz->txrx->protocol_result),
                   "")) {
                strlcpy(
                    subghz->file_name,
                    subghz_protocol_get_last_file_name(
                        (SubGhzProtocolRAW*)subghz->txrx->protocol_result),
                    strlen(subghz_protocol_get_last_file_name(
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
                subghz_protocol_set_last_file_name(
                    (SubGhzProtocolRAW*)subghz->txrx->protocol_result, string_get_cstr(temp_str));
                string_clear(temp_str);

                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSavedMenu);
            }
            return true;
            break;

        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        switch(subghz->state_notifications) {
        case NOTIFICATION_RX_STATE:
            notification_message(subghz->notifications, &sequence_blink_blue_10);
            subghz_save_raw_update_sample_write(
                subghz->subghz_save_raw,
                subghz_save_protocol_raw_get_sample_write(
                    (SubGhzProtocolRAW*)subghz->txrx->protocol_result));
            subghz_save_raw_add_data_rssi(subghz->subghz_save_raw, furi_hal_subghz_get_rssi());
            break;
        default:
            break;
        }
    }
    return false;
}

void subghz_scene_save_raw_on_exit(void* context) {
    SubGhz* subghz = context;

    //Stop CC1101
    if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
        subghz_rx_end(subghz);
        subghz_sleep(subghz);
    };
    subghz->state_notifications = NOTIFICATION_IDLE_STATE;

    //Сallback restoration
    subghz_worker_set_pair_callback(
        subghz->txrx->worker, (SubGhzWorkerPairCallback)subghz_parser_parse);
}
