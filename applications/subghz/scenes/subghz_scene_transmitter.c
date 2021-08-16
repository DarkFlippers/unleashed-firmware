#include "../subghz_i.h"
#include "../views/subghz_transmitter.h"

void subghz_scene_transmitter_callback(SubghzTransmitterEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

const void subghz_scene_transmitter_on_enter(void* context) {
    SubGhz* subghz = context;
    SubghzTransmitter* subghz_transmitter = subghz->subghz_transmitter;

    subghz_transmitter_set_callback(subghz_transmitter, subghz_scene_transmitter_callback, subghz);
    subghz_transmitter_set_protocol(subghz_transmitter, subghz->protocol_result);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewTransmitter);

    subghz->state_notifications = NOTIFICATION_IDLE_STATE;
}

const bool subghz_scene_transmitter_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubghzTransmitterEventSendStart) {
            subghz->state_notifications = NOTIFICATION_TX_STATE;
            subghz_transmitter_tx_start(subghz);
            return true;
        } else if(event.event == SubghzTransmitterEventSendStop) {
            subghz->state_notifications = NOTIFICATION_IDLE_STATE;
            subghz_transmitter_tx_stop(subghz);
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

const void subghz_scene_transmitter_on_exit(void* context) {
    SubGhz* subghz = context;
    SubghzTransmitter* subghz_transmitter = subghz->subghz_transmitter;
    subghz_transmitter_set_callback(subghz_transmitter, NULL, subghz);
    subghz->state_notifications = NOTIFICATION_IDLE_STATE;
}
