#include "../subghz_i.h"
#include "../views/receiver.h"
#include <dolphin/dolphin.h>
#include <lib/subghz/protocols/bin_raw.h>

#define TAG "SubGhzSceneReceiver"

const NotificationSequence subghz_sequence_rx = {
    &message_green_255,

    &message_display_backlight_on,

    &message_vibro_on,
    &message_note_c6,
    &message_delay_50,
    &message_sound_off,
    &message_vibro_off,

    &message_delay_50,
    NULL,
};

const NotificationSequence subghz_sequence_rx_locked = {
    &message_green_255,

    &message_display_backlight_on,

    &message_vibro_on,
    &message_note_c6,
    &message_delay_50,
    &message_sound_off,
    &message_vibro_off,

    &message_delay_500,

    &message_display_backlight_off,
    NULL,
};

static void subghz_scene_receiver_update_statusbar(void* context) {
    SubGhz* subghz = context;
    FuriString* history_stat_str = furi_string_alloc();
    if(!subghz_history_get_text_space_left(subghz->txrx->history, history_stat_str)) {
        FuriString* frequency_str = furi_string_alloc();
        FuriString* modulation_str = furi_string_alloc();

#ifdef SUBGHZ_EXT_PRESET_NAME
        if(subghz_history_get_last_index(subghz->txrx->history) > 0) {
            subghz_get_frequency_modulation(subghz, frequency_str, modulation_str);
        } else {
            subghz_get_frequency_modulation(subghz, frequency_str, NULL);
            furi_string_printf(
                modulation_str,
                "%s        Mod: %s",
                furi_hal_subghz_get_radio_type() ? "Ext" : "Int",
                furi_string_get_cstr(subghz->txrx->preset->name));
        }
#else
        subghz_get_frequency_modulation(subghz, frequency_str, modulation_str);
#endif

        subghz_view_receiver_add_data_statusbar(
            subghz->subghz_receiver,
            furi_string_get_cstr(frequency_str),
            furi_string_get_cstr(modulation_str),
            furi_string_get_cstr(history_stat_str));

        furi_string_free(frequency_str);
        furi_string_free(modulation_str);
    } else {
        subghz_view_receiver_add_data_statusbar(
            subghz->subghz_receiver, furi_string_get_cstr(history_stat_str), "", "");
        subghz->state_notifications = SubGhzNotificationStateIDLE;
    }
    furi_string_free(history_stat_str);
}

void subghz_scene_receiver_callback(SubGhzCustomEvent event, void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, event);
}

static void subghz_scene_add_to_history_callback(
    SubGhzReceiver* receiver,
    SubGhzProtocolDecoderBase* decoder_base,
    void* context) {
    furi_assert(context);

    SubGhz* subghz = context;

    FuriString* item_name = furi_string_alloc();
    FuriString* item_time = furi_string_alloc();
    uint16_t idx = subghz_history_get_item(subghz->txrx->history);

    if(subghz_history_add_to_history(subghz->txrx->history, decoder_base, subghz->txrx->preset)) {
        furi_string_reset(item_name);
        furi_string_reset(item_time);

        subghz->state_notifications = SubGhzNotificationStateRxDone;

        subghz_history_get_text_item_menu(subghz->txrx->history, item_name, idx);
        subghz_history_get_time_item_menu(subghz->txrx->history, item_time, idx);
        subghz_view_receiver_add_item_to_menu(
            subghz->subghz_receiver,
            furi_string_get_cstr(item_name),
            furi_string_get_cstr(item_time),
            subghz_history_get_type_protocol(subghz->txrx->history, idx));

        subghz_scene_receiver_update_statusbar(subghz);
    }
    subghz_receiver_reset(receiver);
    furi_string_free(item_name);
    furi_string_free(item_time);
    subghz->txrx->rx_key_state = SubGhzRxKeyStateAddKey;
}

void subghz_scene_receiver_on_enter(void* context) {
    SubGhz* subghz = context;

    FuriString* item_name = furi_string_alloc();
    FuriString* item_time = furi_string_alloc();

    if(subghz->txrx->rx_key_state == SubGhzRxKeyStateIDLE) {
        subghz_preset_init(subghz, "AM650", subghz->last_settings->frequency, NULL, 0);
        subghz_history_reset(subghz->txrx->history);
        subghz->txrx->rx_key_state = SubGhzRxKeyStateStart;
    }

    subghz_view_receiver_set_lock(subghz->subghz_receiver, subghz->lock);
    subghz_view_receiver_set_mode(subghz->subghz_receiver, SubGhzViewReceiverModeLive);

    //Load history to receiver
    subghz_view_receiver_exit(subghz->subghz_receiver);
    for(uint8_t i = 0; i < subghz_history_get_item(subghz->txrx->history); i++) {
        furi_string_reset(item_name);
        furi_string_reset(item_time);
        subghz_history_get_text_item_menu(subghz->txrx->history, item_name, i);
        subghz_history_get_time_item_menu(subghz->txrx->history, item_time, i);
        subghz_view_receiver_add_item_to_menu(
            subghz->subghz_receiver,
            furi_string_get_cstr(item_name),
            furi_string_get_cstr(item_time),
            subghz_history_get_type_protocol(subghz->txrx->history, i));
        subghz->txrx->rx_key_state = SubGhzRxKeyStateAddKey;
    }
    furi_string_free(item_name);
    furi_string_free(item_time);
    subghz_scene_receiver_update_statusbar(subghz);
    subghz_view_receiver_set_callback(
        subghz->subghz_receiver, subghz_scene_receiver_callback, subghz);
    subghz_receiver_set_rx_callback(
        subghz->txrx->receiver, subghz_scene_add_to_history_callback, subghz);

    // TODO: Replace with proper solution based on protocol flags, remove kostily and velosipedy from here
    // Needs to be done after subghz refactoring merge!!!
    if(subghz->txrx->ignore_starline == true) {
        SubGhzProtocolDecoderBase* protocoldecoderbase = NULL;
        protocoldecoderbase =
            subghz_receiver_search_decoder_base_by_name(subghz->txrx->receiver, "Star Line");
        if(protocoldecoderbase) {
            subghz_protocol_decoder_base_set_decoder_callback(
                protocoldecoderbase, NULL, subghz->txrx->receiver);
        }
    }
    if(subghz->txrx->ignore_auto_alarms == true) {
        SubGhzProtocolDecoderBase* protocoldecoderbase = NULL;
        protocoldecoderbase =
            subghz_receiver_search_decoder_base_by_name(subghz->txrx->receiver, "KIA Seed");
        if(protocoldecoderbase) {
            subghz_protocol_decoder_base_set_decoder_callback(
                protocoldecoderbase, NULL, subghz->txrx->receiver);
        }
        protocoldecoderbase = NULL;
        protocoldecoderbase =
            subghz_receiver_search_decoder_base_by_name(subghz->txrx->receiver, "Scher-Khan");
        if(protocoldecoderbase) {
            subghz_protocol_decoder_base_set_decoder_callback(
                protocoldecoderbase, NULL, subghz->txrx->receiver);
        }
    }
    if(subghz->txrx->ignore_magellan == true) {
        SubGhzProtocolDecoderBase* protocoldecoderbase = NULL;
        protocoldecoderbase =
            subghz_receiver_search_decoder_base_by_name(subghz->txrx->receiver, "Magellan");
        if(protocoldecoderbase) {
            subghz_protocol_decoder_base_set_decoder_callback(
                protocoldecoderbase, NULL, subghz->txrx->receiver);
        }
    }

    subghz->state_notifications = SubGhzNotificationStateRx;
    if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
        subghz_rx_end(subghz);
    }
    if((subghz->txrx->txrx_state == SubGhzTxRxStateIDLE) ||
       (subghz->txrx->txrx_state == SubGhzTxRxStateSleep)) {
        subghz_begin(
            subghz,
            subghz_setting_get_preset_data_by_name(
                subghz->setting, furi_string_get_cstr(subghz->txrx->preset->name)));
        subghz_rx(subghz, subghz->txrx->preset->frequency);
    }
    subghz_view_receiver_set_idx_menu(subghz->subghz_receiver, subghz->txrx->idx_menu_chosen);

    //to use a universal decoder, we are looking for a link to it
    subghz->txrx->decoder_result = subghz_receiver_search_decoder_base_by_name(
        subghz->txrx->receiver, SUBGHZ_PROTOCOL_BIN_RAW_NAME);
    furi_assert(subghz->txrx->decoder_result);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdReceiver);
}

bool subghz_scene_receiver_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SubGhzCustomEventViewReceiverBack:
            // Stop CC1101 Rx
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            if(subghz->txrx->txrx_state == SubGhzTxRxStateRx) {
                subghz_rx_end(subghz);
                subghz_sleep(subghz);
            }
            subghz->txrx->hopper_state = SubGhzHopperStateOFF;
            subghz->txrx->idx_menu_chosen = 0;
            subghz_receiver_set_rx_callback(subghz->txrx->receiver, NULL, subghz);

            if(subghz->txrx->rx_key_state == SubGhzRxKeyStateAddKey) {
                subghz->txrx->rx_key_state = SubGhzRxKeyStateExit;
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneNeedSaving);
            } else {
                subghz->txrx->rx_key_state = SubGhzRxKeyStateIDLE;
                subghz_preset_init(subghz, "AM650", subghz->last_settings->frequency, NULL, 0);
                scene_manager_search_and_switch_to_previous_scene(
                    subghz->scene_manager, SubGhzSceneStart);
            }
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverOK:
            // Show file info, scene: receiver_info
            subghz->txrx->idx_menu_chosen =
                subghz_view_receiver_get_idx_menu(subghz->subghz_receiver);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiverInfo);
            DOLPHIN_DEED(DolphinDeedSubGhzReceiverInfo);
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverDeleteItem:
            subghz->txrx->idx_menu_chosen =
                subghz_view_receiver_get_idx_menu(subghz->subghz_receiver);

            subghz_history_delete_item(subghz->txrx->history, subghz->txrx->idx_menu_chosen);
            subghz_view_receiver_delete_element_callback(subghz->subghz_receiver);

            subghz_scene_receiver_update_statusbar(subghz);
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverConfig:
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            subghz->txrx->idx_menu_chosen =
                subghz_view_receiver_get_idx_menu(subghz->subghz_receiver);
            scene_manager_set_scene_state(
                subghz->scene_manager, SubGhzViewIdReceiver, SubGhzCustomEventManagerSet);
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiverConfig);
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverOffDisplay:
            notification_message(subghz->notifications, &sequence_display_backlight_off);
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverUnlock:
            subghz->lock = SubGhzLockOff;
            consumed = true;
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subghz->txrx->hopper_state != SubGhzHopperStateOFF) {
            subghz_hopper_update(subghz);
            subghz_scene_receiver_update_statusbar(subghz);
        }

        //get RSSI
        float rssi = furi_hal_subghz_get_rssi();
        subghz_receiver_rssi(subghz->subghz_receiver, rssi);
        subghz_protocol_decoder_bin_raw_data_input_rssi(
            (SubGhzProtocolDecoderBinRAW*)subghz->txrx->decoder_result, rssi);

        switch(subghz->state_notifications) {
        case SubGhzNotificationStateRx:
            notification_message(subghz->notifications, &sequence_blink_cyan_10);
            break;
        case SubGhzNotificationStateRxDone:
            if(subghz->lock != SubGhzLockOn) {
                notification_message(subghz->notifications, &subghz_sequence_rx);
            } else {
                notification_message(subghz->notifications, &subghz_sequence_rx_locked);
            }
            subghz->state_notifications = SubGhzNotificationStateRx;
            break;
        default:
            break;
        }
    }
    return consumed;
}

void subghz_scene_receiver_on_exit(void* context) {
    UNUSED(context);
}
