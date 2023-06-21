#include "../pocsag_pager_app_i.h"
#include "../views/pocsag_pager_receiver.h"

static const NotificationSequence subghs_sequence_rx = {
    &message_green_255,

    &message_vibro_on,
    &message_note_c6,
    &message_delay_50,
    &message_sound_off,
    &message_vibro_off,

    &message_delay_50,
    NULL,
};

static const NotificationSequence subghs_sequence_rx_locked = {
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

static void pocsag_pager_scene_receiver_update_statusbar(void* context) {
    POCSAGPagerApp* app = context;
    FuriString* history_stat_str;
    history_stat_str = furi_string_alloc();
    if(!pcsg_history_get_text_space_left(app->txrx->history, history_stat_str)) {
        FuriString* frequency_str;
        FuriString* modulation_str;

        frequency_str = furi_string_alloc();
        modulation_str = furi_string_alloc();

        pcsg_get_frequency_modulation(app, frequency_str, modulation_str);

        pcsg_view_receiver_add_data_statusbar(
            app->pcsg_receiver,
            furi_string_get_cstr(frequency_str),
            furi_string_get_cstr(modulation_str),
            furi_string_get_cstr(history_stat_str));

        furi_string_free(frequency_str);
        furi_string_free(modulation_str);
    } else {
        pcsg_view_receiver_add_data_statusbar(
            app->pcsg_receiver, furi_string_get_cstr(history_stat_str), "", "");
    }
    furi_string_free(history_stat_str);
}

void pocsag_pager_scene_receiver_callback(PCSGCustomEvent event, void* context) {
    furi_assert(context);
    POCSAGPagerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

static void pocsag_pager_scene_receiver_add_to_history_callback(
    SubGhzReceiver* receiver,
    SubGhzProtocolDecoderBase* decoder_base,
    void* context) {
    furi_assert(context);
    POCSAGPagerApp* app = context;
    FuriString* str_buff;
    str_buff = furi_string_alloc();

    if(pcsg_history_add_to_history(app->txrx->history, decoder_base, app->txrx->preset) ==
       PCSGHistoryStateAddKeyNewDada) {
        furi_string_reset(str_buff);

        pcsg_history_get_text_item_menu(
            app->txrx->history, str_buff, pcsg_history_get_item(app->txrx->history) - 1);
        pcsg_view_receiver_add_item_to_menu(
            app->pcsg_receiver,
            furi_string_get_cstr(str_buff),
            pcsg_history_get_type_protocol(
                app->txrx->history, pcsg_history_get_item(app->txrx->history) - 1));

        pocsag_pager_scene_receiver_update_statusbar(app);
        notification_message(app->notifications, &sequence_blink_green_10);
        if(app->lock != PCSGLockOn) {
            notification_message(app->notifications, &subghs_sequence_rx);
        } else {
            notification_message(app->notifications, &subghs_sequence_rx_locked);
        }
    }
    subghz_receiver_reset(receiver);
    furi_string_free(str_buff);
    app->txrx->rx_key_state = PCSGRxKeyStateAddKey;
}

void pocsag_pager_scene_receiver_on_enter(void* context) {
    POCSAGPagerApp* app = context;

    FuriString* str_buff;
    str_buff = furi_string_alloc();

    if(app->txrx->rx_key_state == PCSGRxKeyStateIDLE) {
        pcsg_preset_init(app, "FM95", 439987500, NULL, 0);
        pcsg_history_reset(app->txrx->history);
        app->txrx->rx_key_state = PCSGRxKeyStateStart;
    }

    pcsg_view_receiver_set_lock(app->pcsg_receiver, app->lock);
    pcsg_view_receiver_set_ext_module_state(
        app->pcsg_receiver, radio_device_loader_is_external(app->txrx->radio_device));

    //Load history to receiver
    pcsg_view_receiver_exit(app->pcsg_receiver);
    for(uint8_t i = 0; i < pcsg_history_get_item(app->txrx->history); i++) {
        furi_string_reset(str_buff);
        pcsg_history_get_text_item_menu(app->txrx->history, str_buff, i);
        pcsg_view_receiver_add_item_to_menu(
            app->pcsg_receiver,
            furi_string_get_cstr(str_buff),
            pcsg_history_get_type_protocol(app->txrx->history, i));
        app->txrx->rx_key_state = PCSGRxKeyStateAddKey;
    }
    furi_string_free(str_buff);
    pocsag_pager_scene_receiver_update_statusbar(app);

    pcsg_view_receiver_set_callback(app->pcsg_receiver, pocsag_pager_scene_receiver_callback, app);
    subghz_receiver_set_rx_callback(
        app->txrx->receiver, pocsag_pager_scene_receiver_add_to_history_callback, app);

    if(app->txrx->txrx_state == PCSGTxRxStateRx) {
        pcsg_rx_end(app);
    };
    if((app->txrx->txrx_state == PCSGTxRxStateIDLE) ||
       (app->txrx->txrx_state == PCSGTxRxStateSleep)) {
        // Start RX
        pcsg_begin(
            app,
            subghz_setting_get_preset_data_by_name(
                app->setting, furi_string_get_cstr(app->txrx->preset->name)));

        pcsg_rx(app, app->txrx->preset->frequency);
    }

    pcsg_view_receiver_set_idx_menu(app->pcsg_receiver, app->txrx->idx_menu_chosen);
    view_dispatcher_switch_to_view(app->view_dispatcher, POCSAGPagerViewReceiver);
}

bool pocsag_pager_scene_receiver_on_event(void* context, SceneManagerEvent event) {
    POCSAGPagerApp* app = context;
    bool consumed = false;
    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case PCSGCustomEventViewReceiverBack:
            // Stop CC1101 Rx
            if(app->txrx->txrx_state == PCSGTxRxStateRx) {
                pcsg_rx_end(app);
                pcsg_idle(app);
            };
            app->txrx->hopper_state = PCSGHopperStateOFF;
            app->txrx->idx_menu_chosen = 0;
            subghz_receiver_set_rx_callback(app->txrx->receiver, NULL, app);

            app->txrx->rx_key_state = PCSGRxKeyStateIDLE;
            pcsg_preset_init(app, "FM95", 439987500, NULL, 0);
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, POCSAGPagerSceneStart);
            consumed = true;
            break;
        case PCSGCustomEventViewReceiverOK:
            app->txrx->idx_menu_chosen = pcsg_view_receiver_get_idx_menu(app->pcsg_receiver);
            scene_manager_next_scene(app->scene_manager, POCSAGPagerSceneReceiverInfo);
            consumed = true;
            break;
        case PCSGCustomEventViewReceiverConfig:
            app->txrx->idx_menu_chosen = pcsg_view_receiver_get_idx_menu(app->pcsg_receiver);
            scene_manager_next_scene(app->scene_manager, POCSAGPagerSceneReceiverConfig);
            consumed = true;
            break;
        case PCSGCustomEventViewReceiverOffDisplay:
            notification_message(app->notifications, &sequence_display_backlight_off);
            consumed = true;
            break;
        case PCSGCustomEventViewReceiverUnlock:
            app->lock = PCSGLockOff;
            consumed = true;
            break;
        default:
            break;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        if(app->txrx->hopper_state != PCSGHopperStateOFF) {
            pcsg_hopper_update(app);
            pocsag_pager_scene_receiver_update_statusbar(app);
        }
        // Get current RSSI
        float rssi = subghz_devices_get_rssi(app->txrx->radio_device);
        pcsg_receiver_rssi(app->pcsg_receiver, rssi);

        if(app->txrx->txrx_state == PCSGTxRxStateRx) {
            notification_message(app->notifications, &sequence_blink_cyan_10);
        }
    }
    return consumed;
}

void pocsag_pager_scene_receiver_on_exit(void* context) {
    UNUSED(context);
}
