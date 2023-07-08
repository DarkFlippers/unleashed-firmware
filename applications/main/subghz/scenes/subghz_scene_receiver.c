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
    if(!subghz_history_get_text_space_left(subghz->history, history_stat_str)) {
        FuriString* frequency_str = furi_string_alloc();
        FuriString* modulation_str = furi_string_alloc();

#ifdef SUBGHZ_EXT_PRESET_NAME
        if(subghz_history_get_last_index(subghz->history) > 0) {
            subghz_txrx_get_frequency_and_modulation(
                subghz->txrx, frequency_str, modulation_str, false);
        } else {
            FuriString* temp_str = furi_string_alloc();

            subghz_txrx_get_frequency_and_modulation(subghz->txrx, frequency_str, temp_str, true);
            furi_string_printf(
                modulation_str,
                "%s        Mod: %s",
                (subghz_txrx_radio_device_get(subghz->txrx) == SubGhzRadioDeviceTypeInternal) ?
                    "Int" :
                    "Ext",
                furi_string_get_cstr(temp_str));
            furi_string_free(temp_str);
        }
#else
        subghz_txrx_get_frequency_and_modulation(
            subghz->txrx, frequency_str, modulation_str, false);
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

    subghz_view_receiver_set_radio_device_type(
        subghz->subghz_receiver, subghz_txrx_radio_device_get(subghz->txrx));
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

    // The check can be moved to /lib/subghz/receiver.c, but may result in false positives
    if((decoder_base->protocol->flag & subghz->ignore_filter) == 0) {
        SubGhzHistory* history = subghz->history;
        FuriString* item_name = furi_string_alloc();
        FuriString* item_time = furi_string_alloc();
        uint16_t idx = subghz_history_get_item(history);

        SubGhzRadioPreset preset = subghz_txrx_get_preset(subghz->txrx);
        if(subghz_history_add_to_history(history, decoder_base, &preset)) {
            furi_string_reset(item_name);
            furi_string_reset(item_time);

            subghz->state_notifications = SubGhzNotificationStateRxDone;

            subghz_history_get_text_item_menu(history, item_name, idx);
            subghz_history_get_time_item_menu(history, item_time, idx);
            subghz_view_receiver_add_item_to_menu(
                subghz->subghz_receiver,
                furi_string_get_cstr(item_name),
                furi_string_get_cstr(item_time),
                subghz_history_get_type_protocol(history, idx));

            subghz_scene_receiver_update_statusbar(subghz);
            if(subghz_history_get_text_space_left(subghz->history, NULL)) {
                notification_message(subghz->notifications, &sequence_error);
            }
        }
        subghz_receiver_reset(receiver);
        furi_string_free(item_name);
        furi_string_free(item_time);
        subghz_rx_key_state_set(subghz, SubGhzRxKeyStateAddKey);
    } else {
        FURI_LOG_I(TAG, "%s protocol ignored", decoder_base->protocol->name);
    }
}

void subghz_scene_receiver_on_enter(void* context) {
    SubGhz* subghz = context;
    SubGhzHistory* history = subghz->history;

    FuriString* item_name = furi_string_alloc();
    FuriString* item_time = furi_string_alloc();

    if(subghz_rx_key_state_get(subghz) == SubGhzRxKeyStateIDLE) {
        subghz_txrx_set_preset(subghz->txrx, "AM650", subghz->last_settings->frequency, NULL, 0);
        subghz_history_reset(history);
        subghz_rx_key_state_set(subghz, SubGhzRxKeyStateStart);
        subghz->idx_menu_chosen = 0;
    }

    subghz_view_receiver_set_lock(subghz->subghz_receiver, subghz_is_locked(subghz));
    subghz_view_receiver_set_mode(subghz->subghz_receiver, SubGhzViewReceiverModeLive);

    //Load history to receiver
    subghz_view_receiver_exit(subghz->subghz_receiver);
    for(uint8_t i = 0; i < subghz_history_get_item(history); i++) {
        furi_string_reset(item_name);
        furi_string_reset(item_time);
        subghz_history_get_text_item_menu(history, item_name, i);
        subghz_history_get_time_item_menu(history, item_time, i);
        subghz_view_receiver_add_item_to_menu(
            subghz->subghz_receiver,
            furi_string_get_cstr(item_name),
            furi_string_get_cstr(item_time),
            subghz_history_get_type_protocol(history, i));
        subghz_rx_key_state_set(subghz, SubGhzRxKeyStateAddKey);
    }
    furi_string_free(item_name);
    furi_string_free(item_time);
    subghz_scene_receiver_update_statusbar(subghz);
    subghz_view_receiver_set_callback(
        subghz->subghz_receiver, subghz_scene_receiver_callback, subghz);
    subghz_txrx_set_rx_calback(subghz->txrx, subghz_scene_add_to_history_callback, subghz);

    if(!subghz_history_get_text_space_left(subghz->history, NULL)) {
        subghz->state_notifications = SubGhzNotificationStateRx;
    }
    subghz_txrx_rx_start(subghz->txrx);
    subghz_view_receiver_set_idx_menu(subghz->subghz_receiver, subghz->idx_menu_chosen);

    //to use a universal decoder, we are looking for a link to it
    furi_check(
        subghz_txrx_load_decoder_by_name_protocol(subghz->txrx, SUBGHZ_PROTOCOL_BIN_RAW_NAME));

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdReceiver);
}

bool subghz_scene_receiver_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        // Save cursor position before going to any other dialog
        subghz->idx_menu_chosen = subghz_view_receiver_get_idx_menu(subghz->subghz_receiver);

        switch(event.event) {
        case SubGhzCustomEventViewReceiverBack:
            // Stop CC1101 Rx
            subghz->state_notifications = SubGhzNotificationStateIDLE;
            subghz_txrx_stop(subghz->txrx);
            subghz_txrx_hopper_set_state(subghz->txrx, SubGhzHopperStateOFF);
            subghz_txrx_set_rx_calback(subghz->txrx, NULL, subghz);

            if(subghz_rx_key_state_get(subghz) == SubGhzRxKeyStateAddKey) {
                subghz_rx_key_state_set(subghz, SubGhzRxKeyStateExit);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneNeedSaving);
            } else {
                subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);
                subghz_txrx_set_preset(
                    subghz->txrx, "AM650", subghz->last_settings->frequency, NULL, 0);
                scene_manager_search_and_switch_to_previous_scene(
                    subghz->scene_manager, SubGhzSceneStart);
            }
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverOK:
            // Show file info, scene: receiver_info
            scene_manager_next_scene(subghz->scene_manager, SubGhzSceneReceiverInfo);
            dolphin_deed(DolphinDeedSubGhzReceiverInfo);
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverDeleteItem:
            subghz->state_notifications = SubGhzNotificationStateRx;

            subghz_view_receiver_disable_draw_callback(subghz->subghz_receiver);

            subghz_history_delete_item(subghz->history, subghz->idx_menu_chosen);
            subghz_view_receiver_delete_element_callback(subghz->subghz_receiver);
            subghz_view_receiver_enable_draw_callback(subghz->subghz_receiver);

            subghz_scene_receiver_update_statusbar(subghz);
            subghz->idx_menu_chosen = subghz_view_receiver_get_idx_menu(subghz->subghz_receiver);
            if(subghz_history_get_last_index(subghz->history) == 0) {
                subghz_rx_key_state_set(subghz, SubGhzRxKeyStateStart);
            }
            consumed = true;
            break;
        case SubGhzCustomEventViewReceiverConfig:
            // Actually signals are received but SubGhzNotificationStateRx is not working inside Config Scene
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
            subghz_unlock(subghz);
            consumed = true;
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(subghz_txrx_hopper_get_state(subghz->txrx) != SubGhzHopperStateOFF) {
            subghz_txrx_hopper_update(subghz->txrx);
            subghz_scene_receiver_update_statusbar(subghz);
        }

        SubGhzThresholdRssiData ret_rssi = subghz_threshold_get_rssi_data(
            subghz->threshold_rssi, subghz_txrx_radio_device_get_rssi(subghz->txrx));

        subghz_receiver_rssi(subghz->subghz_receiver, ret_rssi.rssi);
        subghz_protocol_decoder_bin_raw_data_input_rssi(
            (SubGhzProtocolDecoderBinRAW*)subghz_txrx_get_decoder(subghz->txrx), ret_rssi.rssi);

        switch(subghz->state_notifications) {
        case SubGhzNotificationStateRx:
            notification_message(subghz->notifications, &sequence_blink_cyan_10);
            break;
        case SubGhzNotificationStateRxDone:
            if(!subghz_is_locked(subghz)) {
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
