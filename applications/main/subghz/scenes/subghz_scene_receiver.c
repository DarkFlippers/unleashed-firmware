#include "../subghz_i.h"
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

// Four characters of the preset name fit while the count is two digits
static void subghz_scene_receiver_set_modulation(SubGhz* subghz, FuriString* out, size_t count) {
    FuriString* full = furi_string_alloc();
    subghz_txrx_get_frequency_and_modulation(subghz->txrx, NULL, full, true);
    furi_string_set_n(out, full, 0, (count > 2) ? 2 : 4);
    furi_string_free(full);
}

static void subghz_scene_receiver_update_statusbar(void* context) {
    SubGhz* subghz = context;
    FuriString* history_stat_str = furi_string_alloc();
    // The ring keeps making room, so the history never reads as full while it is on
    subghz->history_full = subghz_history_get_text_space_left(
        subghz->history,
        history_stat_str,
        SubGhzProtocolTypeStatic,
        subghz->last_settings->delete_old_signals);
    if(!subghz->history_full) {
        FuriString* frequency_str = furi_string_alloc();
        FuriString* modulation_str = furi_string_alloc();

#ifdef SUBGHZ_EXT_PRESET_NAME
        if(subghz_history_get_last_index(subghz->history) > 0) {
            subghz_txrx_get_frequency_and_modulation(subghz->txrx, frequency_str, NULL, false);
            subghz_scene_receiver_set_modulation(
                subghz, modulation_str, furi_string_size(history_stat_str));
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
        subghz_txrx_get_frequency_and_modulation(subghz->txrx, frequency_str, NULL, false);
        subghz_scene_receiver_set_modulation(
            subghz, modulation_str, furi_string_size(history_stat_str));
#endif

        subghz_view_receiver_add_data_statusbar(
            subghz->subghz_receiver,
            furi_string_get_cstr(frequency_str),
            furi_string_get_cstr(modulation_str),
            furi_string_get_cstr(history_stat_str),
            subghz_txrx_hopper_get_state(subghz->txrx) != SubGhzHopperStateOFF,
            READ_BIT(subghz->filter, SubGhzProtocolFlag_BinRAW) > 0,
            subghz->last_settings->delete_old_signals);

        furi_string_free(frequency_str);
        furi_string_free(modulation_str);
    } else {
        subghz_view_receiver_add_data_statusbar(
            subghz->subghz_receiver,
            furi_string_get_cstr(history_stat_str),
            "",
            "",
            subghz_txrx_hopper_get_state(subghz->txrx) != SubGhzHopperStateOFF,
            READ_BIT(subghz->filter, SubGhzProtocolFlag_BinRAW) > 0,
            subghz->last_settings->delete_old_signals);
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

// Drop the oldest signals until the incoming one fits, one at a time for as long as that
// gets closer to room. While plain heap is what is short it always does. Once a wide enough
// block is what is missing it only does when freed records merge, and a run of small ones
// never will, so the loop stops there rather than empty the list for nothing
static uint16_t subghz_scene_receiver_delete_old_signals(SubGhz* subghz, uint8_t type) {
    uint16_t deleted = 0;
    size_t widest = 0;
    while((subghz_history_get_last_index(subghz->history) > 0) &&
          subghz_history_is_full(subghz->history, type)) {
        const size_t free_block = memmgr_heap_get_max_free_block();
        if((deleted != 0) && (free_block <= widest) &&
           !subghz_history_needs_heap(subghz->history, type)) {
            break;
        }
        widest = free_block;
        if(deleted == 0) {
            subghz_view_receiver_disable_draw_callback(subghz->subghz_receiver);
        }
        subghz_history_delete_item(subghz->history, 0);
        subghz_view_receiver_delete_item(subghz->subghz_receiver, 0);
        deleted++;
    }
    if(deleted != 0) {
        subghz_view_receiver_enable_draw_callback(subghz->subghz_receiver);
        subghz_scene_receiver_update_statusbar(subghz);
        subghz->idx_menu_chosen = subghz_view_receiver_get_idx_menu(subghz->subghz_receiver);
    }
    return deleted;
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

        const bool ring = subghz->last_settings->delete_old_signals;
        SubGhzRadioPreset preset = subghz_txrx_get_preset(subghz->txrx);
        if(ring) {
            uint16_t deleted =
                subghz_scene_receiver_delete_old_signals(subghz, decoder_base->protocol->type);
            if(deleted != 0) {
                // Only lift the idle a full history left behind. The beep plays from the scene
                // tick, so a signal that landed less than a tick ago is still waiting on
                // RxDone - and every repeat of its burst comes through here and deletes again,
                // which is what used to eat it
                if(subghz->state_notifications == SubGhzNotificationStateIDLE) {
                    subghz->state_notifications = SubGhzNotificationStateRx;
                }
                idx -= deleted;
            }
        }
        if(subghz_history_add_to_history(
               history, decoder_base, &preset, subghz_txrx_get_air_time_ms(subghz->txrx))) {
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
            if(subghz_history_get_text_space_left(
                   subghz->history, NULL, SubGhzProtocolTypeStatic, ring)) {
                notification_message(subghz->notifications, &sequence_error);
            }
            subghz_rx_key_state_set(subghz, SubGhzRxKeyStateAddKey);
        } else if(subghz_history_is_full(history, decoder_base->protocol->type)) {
            // The bar is only redrawn after a signal lands, so warn from here too. Every
            // refused signal buzzes, not just the first, so a full history is felt. Still full
            // with the ring on means deleting could not free enough - the signal is lost just
            // the same, but the next one may well fit, so RX is left running
            if(!ring) {
                subghz->state_notifications = SubGhzNotificationStateIDLE;
            }
            subghz_scene_receiver_update_statusbar(subghz);
            notification_message(subghz->notifications, &sequence_error);
        }
        subghz_receiver_reset(receiver);
        furi_string_free(item_name);
        furi_string_free(item_time);
    } else {
        FURI_LOG_D(TAG, "%s protocol ignored", decoder_base->protocol->name);
    }
}

void subghz_scene_receiver_on_enter(void* context) {
    SubGhz* subghz = context;
    SubGhzHistory* history = subghz->history;

    FuriString* item_name = furi_string_alloc();
    FuriString* item_time = furi_string_alloc();

    if(subghz_rx_key_state_get(subghz) == SubGhzRxKeyStateIDLE) {
        subghz_txrx_set_preset_internal(
            subghz->txrx,
            subghz->last_settings->frequency,
            subghz->last_settings->preset_index,
            subghz->last_settings->tx_power);

        subghz->filter = subghz->last_settings->filter;
        subghz_txrx_receiver_set_filter(subghz->txrx, subghz->filter);
        subghz->ignore_filter = subghz->last_settings->ignore_filter;
        subghz->tx_power = subghz->last_settings->tx_power;

        subghz_history_reset(history);
        subghz_rx_key_state_set(subghz, SubGhzRxKeyStateStart);
        subghz->idx_menu_chosen = 0;
    }

    subghz_view_receiver_set_mode(subghz->subghz_receiver, SubGhzViewReceiverModeLive);

    // Load history to receiver
    subghz_view_receiver_exit(subghz->subghz_receiver);
    for(uint16_t i = 0; i < subghz_history_get_item(history); i++) {
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

    subghz_view_receiver_set_callback(
        subghz->subghz_receiver, subghz_scene_receiver_callback, subghz);
    subghz_txrx_set_rx_callback(subghz->txrx, subghz_scene_add_to_history_callback, subghz);

    if(!subghz->history_full) {
        subghz->state_notifications = SubGhzNotificationStateRx;
    }

    // Check if hopping was enabled
    if(subghz->last_settings->enable_hopping) {
        subghz_txrx_hopper_set_state(subghz->txrx, SubGhzHopperStateRunning);
    } else {
        subghz_txrx_hopper_set_state(subghz->txrx, SubGhzHopperStateOFF);
    }

    subghz_txrx_rx_start(subghz->txrx);

    //this scene is re-entered every time a scene on top of it is closed, and RX is
    //restarted from scratch above - drop whatever the decoders were in the middle of
    //when RX went down
    subghz_receiver_reset(subghz_txrx_get_receiver(subghz->txrx));

    subghz_view_receiver_set_idx_menu(subghz->subghz_receiver, subghz->idx_menu_chosen);

    //to use a universal decoder, we are looking for a link to it
    furi_check(
        subghz_txrx_load_decoder_by_name_protocol(subghz->txrx, SUBGHZ_PROTOCOL_BIN_RAW_NAME));

    subghz_scene_receiver_update_statusbar(subghz);

    subghz_view_receiver_set_lock(subghz->subghz_receiver, subghz_is_locked(subghz));

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
            subghz_txrx_set_rx_callback(subghz->txrx, NULL, subghz);

            if(subghz_rx_key_state_get(subghz) == SubGhzRxKeyStateAddKey) {
                subghz_rx_key_state_set(subghz, SubGhzRxKeyStateExit);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneNeedSaving);
            } else {
                subghz_rx_key_state_set(subghz, SubGhzRxKeyStateIDLE);
                subghz_txrx_set_default_preset(subghz->txrx, subghz->last_settings->frequency);
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
            subghz_view_receiver_delete_item(
                subghz->subghz_receiver,
                subghz_view_receiver_get_idx_menu(subghz->subghz_receiver));
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
        //a module unplugged while this screen is up is noticed here or not at all
        bool redraw = subghz_txrx_radio_device_poll_active(subghz->txrx);

        if(subghz_txrx_hopper_get_state(subghz->txrx) != SubGhzHopperStateOFF) {
            subghz_txrx_hopper_update(subghz->txrx, subghz->last_settings->hopping_threshold);
            redraw = true;
        }
        if(redraw) {
            subghz_scene_receiver_update_statusbar(subghz);
        }

        SubGhzThresholdRssiData ret_rssi = subghz_threshold_get_rssi_data(
            subghz->threshold_rssi, subghz_txrx_radio_device_get_rssi(subghz->txrx));

        subghz_receiver_rssi(subghz->subghz_receiver, ret_rssi.rssi);
        // Feeding it is what makes it take its 10k of capture buffers
        if(READ_BIT(subghz->filter, SubGhzProtocolFlag_BinRAW)) {
            subghz_protocol_decoder_bin_raw_data_input_rssi(
                (SubGhzProtocolDecoderBinRAW*)subghz_txrx_get_decoder(subghz->txrx),
                ret_rssi.rssi);
        }

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
