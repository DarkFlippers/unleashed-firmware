#include "../subghz_i.h"
#include "../views/subghz_transmitter.h"
#include <lib/subghz/protocols/subghz_protocol_keeloq.h>

void subghz_scene_transmitter_callback(SubghzTransmitterEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

static void subghz_scene_transmitter_update_data_show(void* context) {
    SubGhz* subghz = context;

    if(subghz->txrx->protocol_result && subghz->txrx->protocol_result->get_upload_protocol) {
        string_t key_str;
        string_init(key_str);
        char frequency_str[10];
        char preset_str[6];
        uint8_t show_button = 0;
        subghz->txrx->protocol_result->to_string(subghz->txrx->protocol_result, key_str);

        if((!strcmp(subghz->txrx->protocol_result->name, "KeeLoq")) &&
           (!strcmp(
               subghz_protocol_keeloq_get_manufacture_name(subghz->txrx->protocol_result),
               "Unknown"))) {
            show_button = 0;
        } else {
            show_button = 1;
        }
        snprintf(
            frequency_str,
            sizeof(frequency_str),
            "%03ld.%02ld",
            subghz->txrx->frequency / 1000000 % 1000,
            subghz->txrx->frequency / 10000 % 100);
        if(subghz->txrx->preset == FuriHalSubGhzPresetOok650Async ||
           subghz->txrx->preset == FuriHalSubGhzPresetOok270Async) {
            snprintf(preset_str, sizeof(preset_str), "AM");
        } else if(subghz->txrx->preset == FuriHalSubGhzPreset2FSKAsync) {
            snprintf(preset_str, sizeof(preset_str), "FM");
        } else {
            furi_crash(NULL);
        }

        subghz_transmitter_add_data_to_show(
            subghz->subghz_transmitter,
            string_get_cstr(key_str),
            frequency_str,
            preset_str,
            show_button);
        string_clear(key_str);
    } else {
        string_set(subghz->error_str, "Protocol not found");
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
    }
}

void subghz_scene_transmitter_on_enter(void* context) {
    SubGhz* subghz = context;
    subghz_transmitter_set_callback(
        subghz->subghz_transmitter, subghz_scene_transmitter_callback, subghz);
    subghz_scene_transmitter_update_data_show(subghz);
    subghz->state_notifications = NOTIFICATION_IDLE_STATE;
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewTransmitter);
}

bool subghz_scene_transmitter_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubghzTransmitterEventSendStart) {
            subghz->state_notifications = NOTIFICATION_IDLE_STATE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
            }
            if((subghz->txrx->txrx_state == SubGhzTxRxStateIdle) ||
               (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
                if(!subghz_tx_start(subghz)) {
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowOnlyRx);
                } else {
                    subghz->state_notifications = NOTIFICATION_TX_STATE;
                    subghz_scene_transmitter_update_data_show(subghz);
                }
            }
            return true;
        } else if(event.event == SubghzTransmitterEventSendStop) {
            subghz->state_notifications = NOTIFICATION_IDLE_STATE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
            }
            return true;
        } else if(event.event == SubghzTransmitterEventBack) {
            subghz->state_notifications = NOTIFICATION_IDLE_STATE;
            scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneStart);
            return true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subghz->state_notifications == NOTIFICATION_TX_STATE) {
            notification_message(subghz->notifications, &sequence_blink_red_10);
        }
        return true;
    }
    return false;
}

void subghz_scene_transmitter_on_exit(void* context) {
    SubGhz* subghz = context;

    subghz->state_notifications = NOTIFICATION_IDLE_STATE;
}
