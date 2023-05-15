#include "../subghz_remote_app_i.h"
#include "../views/transmitter.h"

#include <lib/subghz/protocols/raw.h>

// TODO:
// #include <lib/subghz/protocols/keeloq.h>
// #include <lib/subghz/protocols/star_line.h>

// #include <lib/subghz/blocks/custom_btn.h>

void subrem_scene_remote_callback(SubRemCustomEvent event, void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

void subrem_scene_remote_raw_callback_end_tx(void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SubRemCustomEventViewRemoteForceStop);
}

bool subrem_scene_remote_update_data_show(void* context) {
    SubGhzRemoteApp* app = context;
    //UNUSED(app);
    bool ret = false;

    subrem_view_remote_add_data_to_show(
        //app->subrem_remote_view, "N/A", "N/A", "N/A", "N/A", "N/A");
        app->subrem_remote_view,
        // "UP",
        // "DOWN",
        // "LEFT",
        // "RIGHT",
        // "OK");

        furi_string_get_cstr(app->subs_preset[0]->label),
        furi_string_get_cstr(app->subs_preset[1]->label),
        furi_string_get_cstr(app->subs_preset[2]->label),
        furi_string_get_cstr(app->subs_preset[3]->label),
        furi_string_get_cstr(app->subs_preset[4]->label));

    // SubGhzProtocolDecoderBase* decoder = subghz_txrx_get_decoder(app->txrx);

    // if(decoder) {
    //     FuriString* key_str = furi_string_alloc();
    //     FuriString* frequency_str = furi_string_alloc();
    //     FuriString* modulation_str = furi_string_alloc();

    //     if(subghz_protocol_decoder_base_deserialize(
    //            decoder, subghz_txrx_get_fff_data(app->txrx)) == SubGhzProtocolStatusOk) {
    //         subghz_protocol_decoder_base_get_string(decoder, key_str);

    //         subghz_txrx_get_frequency_and_modulation(
    //             app->txrx, frequency_str, modulation_str, false);
    //         subghz_view_transmitter_add_data_to_show(
    //             app->subghz_transmitter,
    //             furi_string_get_cstr(key_str),
    //             furi_string_get_cstr(frequency_str),
    //             furi_string_get_cstr(modulation_str),
    //             subghz_txrx_protocol_is_transmittable(app->txrx, false));
    //         ret = true;
    //     }
    //     furi_string_free(frequency_str);
    //     furi_string_free(modulation_str);
    //     furi_string_free(key_str);
    // }
    return ret;
}

void subrem_scene_remote_on_enter(void* context) {
    SubGhzRemoteApp* app = context;

    // TODO: reset custom btns
    // keeloq_reset_original_btn();
    // subghz_custom_btns_reset();

    // TODO: init view data

    if(!subrem_scene_remote_update_data_show(app)) {
        // view_dispatcher_send_custom_event(
        //     app->view_dispatcher, SubGhzCustomEventViewTransmitterError);
    }
    subrem_view_remote_set_callback(app->subrem_remote_view, subrem_scene_remote_callback, app);

    // TODO: notifications
    // app->state_notifications = SubGhzNotificationStateIDLE;
    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDRemote);
}

bool subrem_scene_remote_on_event(void* context, SceneManagerEvent event) {
    SubGhzRemoteApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        // if(event.event == SubGhzCustomEventViewTransmitterSendStart) {
        //     app->state_notifications = SubGhzNotificationStateIDLE;

        //     if(subghz_tx_start(app, subghz_txrx_get_fff_data(app->txrx))) {
        //         app->state_notifications = SubGhzNotificationStateTx;
        //         subrem_scene_remote_update_data_show(app);
        //         DOLPHIN_DEED(DolphinDeedSubGhzSend);
        //     }
        //     return true;
        // } else if(event.event == SubGhzCustomEventViewTransmitterSendStop) {
        //     app->state_notifications = SubGhzNotificationStateIDLE;
        //     subghz_txrx_stop(app->txrx);
        //     if(subghz_custom_btn_get() != 0) {
        //         subghz_custom_btn_set(0);
        //         uint8_t tmp_counter = furi_hal_subghz_get_rolling_counter_mult();
        //         furi_hal_subghz_set_rolling_counter_mult(0);
        //         // Calling restore!
        //         subghz_tx_start(app, subghz_txrx_get_fff_data(app->txrx));
        //         subghz_txrx_stop(app->txrx);
        //         furi_hal_subghz_set_rolling_counter_mult(tmp_counter);
        //     }
        //     return true;
        // } else
        if(event.event == SubRemCustomEventViewRemoteBack) {
            // app->state_notifications = SubGhzNotificationStateIDLE; //TODO: notification
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, SubRemSceneStart);
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteStartUP) {
            if(subghz_tx_start_sub(
                   app, app->subs_preset[0], subrem_scene_remote_raw_callback_end_tx)) {
                app->chusen_sub = 0;
                subrem_view_remote_set_state(app->subrem_remote_view, 1);
                notification_message(app->notifications, &sequence_blink_start_magenta);
            }
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteStartDOWN) {
            if(subghz_tx_start_sub(
                   app, app->subs_preset[1], subrem_scene_remote_raw_callback_end_tx)) {
                app->chusen_sub = 1;
                subrem_view_remote_set_state(app->subrem_remote_view, 2);
                notification_message(app->notifications, &sequence_blink_start_magenta);
            }
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteStartLEFT) {
            if(subghz_tx_start_sub(
                   app, app->subs_preset[2], subrem_scene_remote_raw_callback_end_tx)) {
                app->chusen_sub = 2;
                subrem_view_remote_set_state(app->subrem_remote_view, 3);
                notification_message(app->notifications, &sequence_blink_start_magenta);
            }
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteStartRIGHT) {
            if(subghz_tx_start_sub(
                   app, app->subs_preset[3], subrem_scene_remote_raw_callback_end_tx)) {
                app->chusen_sub = 3;
                subrem_view_remote_set_state(app->subrem_remote_view, 4);
                notification_message(app->notifications, &sequence_blink_start_magenta);
            }
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteStartOK) {
            if(subghz_tx_start_sub(
                   app, app->subs_preset[4], subrem_scene_remote_raw_callback_end_tx)) {
                app->chusen_sub = 4;
                subrem_view_remote_set_state(app->subrem_remote_view, 5);
                notification_message(app->notifications, &sequence_blink_start_magenta);
            }
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteForceStop) {
            if(app->tx_running) {
                subghz_tx_stop_sub(app);
                subrem_view_remote_set_state(app->subrem_remote_view, 0);
            }
            notification_message(app->notifications, &sequence_blink_stop);
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteStop) {
            // if(app->tx_running &&
            //    (app->subs_preset[app->chusen_sub]->type == SubRemSubKeyTypeRawKey)) {
            //     subghz_tx_stop_sub(app);
            //     subrem_view_remote_set_state(app->subrem_remote_view, 0);
            // }
            // notification_message(app->notifications, &sequence_blink_stop);
            if(app->tx_running) {
                subghz_tx_stop_sub(app);
                subrem_view_remote_set_state(app->subrem_remote_view, 0);
            }
            notification_message(app->notifications, &sequence_blink_stop);
            return true;
        }
        // notification_message(app->notification, &sequence_blink_stop);

        // else if(event.event == SubGhzCustomEventViewTransmitterError) {
        //     furi_string_set(app->error_str, "Protocol not\nfound!");
        //     scene_manager_next_scene(app->scene_manager, SubGhzSceneShowErrorSub);
        // }
    } else if(event.type == SceneManagerEventTypeTick) {
        // if(app->state_notifications == SubGhzNotificationStateTx) {
        //     notification_message(app->notifications, &sequence_blink_magenta_10);
        // }
        // return true;
    }
    return false;
}

void subrem_scene_remote_on_exit(void* context) {
    SubGhzRemoteApp* app = context;
    UNUSED(app);
    // TODO: notifications and reset KL

    //app->state_notifications = SubGhzNotificationStateIDLE;

    // keeloq_reset_mfname();
    // keeloq_reset_kl_type();
    // keeloq_reset_original_btn();
    // subghz_custom_btns_reset();
    // star_line_reset_mfname();
    // star_line_reset_kl_type();
}
