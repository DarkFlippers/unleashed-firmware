#include "../subghz_i.h"
#include "../views/subghz_receiver.h"

static void subghz_scene_receiver_update_statusbar(void* context) {
    SubGhz* subghz = context;
    string_t history_stat_str;
    string_init(history_stat_str);
    if(!subghz_history_get_text_space_left(subghz->txrx->history, history_stat_str)) {
        string_t frequency_str;
        string_t modulation_str;

        string_init(frequency_str);
        string_init(modulation_str);

        subghz_get_frequency_modulation(subghz, frequency_str, modulation_str);

        subghz_receiver_add_data_statusbar(
            subghz->subghz_receiver,
            string_get_cstr(frequency_str),
            string_get_cstr(modulation_str),
            string_get_cstr(history_stat_str));

        string_clear(frequency_str);
        string_clear(modulation_str);
    } else {
        subghz_receiver_add_data_statusbar(
            subghz->subghz_receiver, string_get_cstr(history_stat_str), "", "");
        subghz->state_notifications = SubGhzNotificationStateIDLE;
    }
    string_clear(history_stat_str);
}

void subghz_scene_receiver_callback(SubghzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

void subghz_scene_add_to_history_callback(SubGhzProtocolCommon* parser, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    string_t str_buff;
    string_init(str_buff);

    if(subghz_history_add_to_history(
           subghz->txrx->history, parser, subghz->txrx->frequency, subghz->txrx->preset)) {
        subghz_parser_reset(subghz->txrx->parser);
        string_reset(str_buff);
        subghz_history_get_text_item_menu(
            subghz->txrx->history, str_buff, subghz_history_get_item(subghz->txrx->history) - 1);
        subghz_receiver_add_item_to_menu(
            subghz->subghz_receiver,
            string_get_cstr(str_buff),
            subghz_history_get_type_protocol(
                subghz->txrx->history, subghz_history_get_item(subghz->txrx->history) - 1));
        subghz_scene_receiver_update_statusbar(subghz);
    }
    string_clear(str_buff);
}

void subghz_scene_receiver_on_enter(void* context) {
    SubGhz* subghz = context;

    string_t str_buff;
    string_init(str_buff);

    //Load history to receiver
    subghz_receiver_exit(subghz->subghz_receiver);
    for(uint8_t i = 0; i < subghz_history_get_item(subghz->txrx->history); i++) {
        string_reset(str_buff);
        subghz_history_get_text_item_menu(subghz->txrx->history, str_buff, i);
        subghz_receiver_add_item_to_menu(
            subghz->subghz_receiver,
            string_get_cstr(str_buff),
            subghz_history_get_type_protocol(subghz->txrx->history, i));
    }
    string_clear(str_buff);
    subghz_scene_receiver_update_statusbar(subghz);
    subghz_receiver_set_callback(subghz->subghz_receiver, subghz_scene_receiver_callback, subghz);
    subghz_parser_enable_dump(subghz->txrx->parser, subghz_scene_add_to_history_callback, subghz);

    subghz->state_notifications = SubGhzNotificationStateRX;
    if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
        subghz_rx_end(subghz);
    };
    if((subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) ||
       (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
        subghz_begin(subghz, subghz->txrx->preset);
        subghz_rx(subghz, subghz->txrx->frequency);
    }
    subghz_receiver_set_idx_menu(subghz->subghz_receiver, subghz->txrx->idx_menu_chosen);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewReceiver);
}

bool subghz_scene_receiver_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SubghzCustomEventViewReceverBack:
            // Stop CC1101 Rx
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            };
            subghz_history_clean(subghz->txrx->history);
            subghz->txrx->hopper_state = SubGhzHopperStateOFF;
            subghz->txrx->frequency = subghz_frequencies[subghz_frequencies_433_92];
            subghz->txrx->preset = FuriHalSubGhzPresetOok650Async;
            subghz->txrx->idx_menu_chosen = 0;
            subghz_parser_enable_dump(subghz->txrx->parser, NULL, subghz);
            scene_manager_search_and_switch_to_previous_scene(
                subghz->scene_manager, SubGhzSceneStart);
            return true;
            break;
        case SubghzCustomEventViewReceverOK:
            subghz->txrx->idx_menu_chosen = subghz_receiver_get_idx_menu(subghz->subghz_receiver);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiverInfo);
            return true;
            break;
        case SubghzCustomEventViewReceverConfig:
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            subghz->txrx->idx_menu_chosen = subghz_receiver_get_idx_menu(subghz->subghz_receiver);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiverConfig);
            return true;
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subghz->txrx->hopper_state != SubGhzHopperStateOFF) {
            subghz_hopper_update(subghz);
            subghz_scene_receiver_update_statusbar(subghz);
        }

        switch(subghz->state_notifications) {
        case SubGhzNotificationStateRX:
            notification_message(subghz->notifications, &sequence_blink_blue_10);
            break;
        default:
            break;
        }
    }
    return false;
}

void subghz_scene_receiver_on_exit(void* context) {
    // SubGhz* subghz = context;
}
