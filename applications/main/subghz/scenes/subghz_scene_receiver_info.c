#include "../subghz_i.h"
#include "../helpers/subghz_custom_event.h"
#include "../views/transmitter.h"

void subghz_scene_receiver_info_callback(SubGhzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

static bool subghz_scene_receiver_info_update_parser(void* context) {
    SubGhz* subghz = context;

    if(subghz_txrx_load_decoder_by_name_protocol(
           subghz->txrx,
           subghz_history_get_protocol_name(subghz->history, subghz->idx_menu_chosen))) {
        // we are trying to deserialize without checking for errors, since it is assumed that we just received this chignal
        subghz_protocol_decoder_base_deserialize(
            subghz_txrx_get_decoder(subghz->txrx),
            subghz_history_get_raw_data(subghz->history, subghz->idx_menu_chosen));

        SubGhzRadioPreset* preset =
            subghz_history_get_radio_preset(subghz->history, subghz->idx_menu_chosen);
        subghz_txrx_set_preset(
            subghz->txrx,
            furi_string_get_cstr(preset->name),
            preset->frequency,
            preset->data,
            preset->data_size);

        FuriString* key_str = furi_string_alloc();
        FuriString* frequency_str = furi_string_alloc();
        FuriString* modulation_str = furi_string_alloc();

        subghz_protocol_decoder_base_get_string(subghz_txrx_get_decoder(subghz->txrx), key_str);
        subghz_txrx_get_frequency_and_modulation(subghz->txrx, frequency_str, modulation_str);
        subghz_view_transmitter_add_data_to_show(
            subghz->subghz_transmitter,
            furi_string_get_cstr(key_str),
            furi_string_get_cstr(frequency_str),
            furi_string_get_cstr(modulation_str),
            subghz_txrx_protocol_is_transmittable(subghz->txrx, true));

        furi_string_free(frequency_str);
        furi_string_free(modulation_str);
        furi_string_free(key_str);

        subghz_view_transmitter_set_radio_device_type(
            subghz->subghz_transmitter, subghz_txrx_radio_device_get(subghz->txrx));
        subghz_view_transmitter_set_model_type(
            subghz->subghz_transmitter, SubGhzViewTransmitterModelTypeInfo);

        return true;
    }
    return false;
}

void subghz_scene_receiver_info_on_enter(void* context) {
    SubGhz* subghz = context;
    if(subghz_scene_receiver_info_update_parser(subghz)) {
    } else {
        view_dispatcher_send_custom_event(
            subghz->view_dispatcher, SubGhzCustomEventSceneShowErrorSub);
    }

    subghz_view_transmitter_set_callback(
        subghz->subghz_transmitter, subghz_scene_receiver_info_callback, subghz);

    subghz->state_notifications = SubGhzNotificationStateIDLE;
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdTransmitter);
}

bool subghz_scene_receiver_info_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventViewTransmitterSendStart) {
            if(!subghz_scene_receiver_info_update_parser(subghz)) {
                return false;
            }
            //CC1101 Stop RX -> Start TX
            subghz_txrx_hopper_pause(subghz->txrx);
            if(!subghz_tx_start(
                   subghz,
                   subghz_history_get_raw_data(subghz->history, subghz->idx_menu_chosen))) {
                subghz_txrx_rx_start(subghz->txrx);
                subghz_txrx_hopper_unpause(subghz->txrx);
                subghz->state_notifications = SubGhzNotificationStateRx;
            } else {
                subghz->state_notifications = SubGhzNotificationStateTx;
            }
            return true;
        } else if(event.event == SubGhzCustomEventViewTransmitterSendStop) {
            //CC1101 Stop Tx -> Start RX
            subghz->state_notifications = SubGhzNotificationStateIDLE;

            subghz_txrx_rx_start(subghz->txrx);

            subghz_txrx_hopper_unpause(subghz->txrx);
            subghz->state_notifications = SubGhzNotificationStateRx;
            return true;
        } else if(event.event == SubGhzCustomEventViewTransmitterSendSave) {
            //CC1101 Stop RX -> Save
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            subghz_txrx_hopper_set_state(subghz->txrx, SubGhzHopperStateOFF);

            subghz_txrx_stop(subghz->txrx);
            if(!subghz_scene_receiver_info_update_parser(subghz)) {
                return false;
            }

            if(subghz_txrx_protocol_is_serializable(subghz->txrx)) {
                subghz_file_name_clear(subghz);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveName);
            }
            return true;
        } else if(event.event == SubGhzCustomEventSceneShowErrorSub) {
            furi_string_set(subghz->error_str, "Error history parse.");
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
        }

    } else if(event.type == SceneManagerEventTypeTick) {
        if(subghz_txrx_hopper_get_state(subghz->txrx) != SubGhzHopperStateOFF) {
            subghz_txrx_hopper_update(subghz->txrx);
        }
        switch(subghz->state_notifications) {
        case SubGhzNotificationStateTx:
            notification_message(subghz->notifications, &sequence_blink_magenta_10);
            break;
        case SubGhzNotificationStateRx:
            notification_message(subghz->notifications, &sequence_blink_cyan_10);
            break;
        case SubGhzNotificationStateRxDone:
            notification_message(subghz->notifications, &sequence_blink_green_100);
            subghz->state_notifications = SubGhzNotificationStateRx;
            break;
        default:
            break;
        }
    }
    return false;
}

void subghz_scene_receiver_info_on_exit(void* context) {
    SubGhz* subghz = context;
    widget_reset(subghz->widget);
}
