#include "../subghz_i.h"
#include "../views/subghz_transmitter.h"
#include <lib/subghz/protocols/subghz_protocol_keeloq.h>
#include <dolphin/dolphin.h>

void subghz_scene_transmitter_callback(SubghzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

bool subghz_scene_transmitter_update_data_show(void* context) {
    SubGhz* subghz = context;

    if(subghz->txrx->protocol_result && subghz->txrx->protocol_result->get_upload_protocol) {
        string_t key_str;
        string_t frequency_str;
        string_t modulation_str;

        string_init(key_str);
        string_init(frequency_str);
        string_init(modulation_str);
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

        subghz_get_frequency_modulation(subghz, frequency_str, modulation_str);
        subghz_transmitter_add_data_to_show(
            subghz->subghz_transmitter,
            string_get_cstr(key_str),
            string_get_cstr(frequency_str),
            string_get_cstr(modulation_str),
            show_button);

        string_clear(frequency_str);
        string_clear(modulation_str);
        string_clear(key_str);

        return true;
    }
    return false;
}

void subghz_scene_transmitter_on_enter(void* context) {
    SubGhz* subghz = context;
    DOLPHIN_DEED(DolphinDeedSubGhzSend);
    if(!subghz_scene_transmitter_update_data_show(subghz)) {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubghzCustomEventViewTransmitterError);
    }

    subghz_transmitter_set_callback(
        subghz->subghz_transmitter, subghz_scene_transmitter_callback, subghz);

    subghz->state_notifications = SubGhzNotificationStateIDLE;
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewTransmitter);
}

bool subghz_scene_transmitter_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubghzCustomEventViewTransmitterSendStart) {
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
            }
            if((subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) ||
               (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
                if(!subghz_tx_start(subghz)) {
                    scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowOnlyRx);
                } else {
                    subghz->state_notifications = SubGhzNotificationStateTX;
                    subghz_scene_transmitter_update_data_show(subghz);
                }
            }
            return true;
        } else if(event.event == SubghzCustomEventViewTransmitterSendStop) {
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateTx) {
                subghz_tx_stop(subghz);
                subghz_sleep(subghz);
            }
            return true;
        } else if(event.event == SubghzCustomEventViewTransmitterBack) {
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneStart);
            return true;
        } else if(event.event == SubghzCustomEventViewTransmitterError) {
            string_set(subghz->error_str, "Protocol not found");
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subghz->state_notifications == SubGhzNotificationStateTX) {
            notification_message(subghz->notifications, &sequence_blink_red_10);
        }
        return true;
    }
    return false;
}

void subghz_scene_transmitter_on_exit(void* context) {
    SubGhz* subghz = context;

    subghz->state_notifications = SubGhzNotificationStateIDLE;
}
