#include "../subghz_remote_app_i.h"
#include "../views/remote.h"

#include <lib/subghz/protocols/raw.h>

#define TAG "SubRemScenRemote"

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
    view_dispatcher_send_custom_event(app->view_dispatcher, SubRemCustomEventViewRemoteForcedStop);
}

static uint8_t subrem_scene_remote_event_to_index(SubRemCustomEvent event_id) {
    uint8_t ret = 0;

    if(event_id == SubRemCustomEventViewRemoteStartUP) {
        ret = SubRemSubKeyNameUp;
    } else if(event_id == SubRemCustomEventViewRemoteStartDOWN) {
        ret = SubRemSubKeyNameDown;
    } else if(event_id == SubRemCustomEventViewRemoteStartLEFT) {
        ret = SubRemSubKeyNameLeft;
    } else if(event_id == SubRemCustomEventViewRemoteStartRIGHT) {
        ret = SubRemSubKeyNameRight;
    } else if(event_id == SubRemCustomEventViewRemoteStartOK) {
        ret = SubRemSubKeyNameOk;
    }

    return ret;
}

static bool subrem_scene_remote_update_data_show(void* context) {
    SubGhzRemoteApp* app = context;
    bool ret = false;

    subrem_view_remote_add_data_to_show(
        app->subrem_remote_view,
        furi_string_get_cstr(app->subs_preset[0]->label),
        furi_string_get_cstr(app->subs_preset[1]->label),
        furi_string_get_cstr(app->subs_preset[2]->label),
        furi_string_get_cstr(app->subs_preset[3]->label),
        furi_string_get_cstr(app->subs_preset[4]->label));

    return ret;
}

void subrem_scene_remote_on_enter(void* context) {
    SubGhzRemoteApp* app = context;

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
        if(event.event == SubRemCustomEventViewRemoteBack) {
            if(!scene_manager_search_and_switch_to_previous_scene(
                   app->scene_manager, SubRemSceneOpenMapFile)) {
                if(!scene_manager_search_and_switch_to_previous_scene(
                       app->scene_manager, SubRemSceneStart)) {
                    scene_manager_stop(app->scene_manager);
                    view_dispatcher_stop(app->view_dispatcher);
                }
            }
            return true;
        } else if(
            event.event == SubRemCustomEventViewRemoteStartUP ||
            event.event == SubRemCustomEventViewRemoteStartDOWN ||
            event.event == SubRemCustomEventViewRemoteStartLEFT ||
            event.event == SubRemCustomEventViewRemoteStartRIGHT ||
            event.event == SubRemCustomEventViewRemoteStartOK) {
            // Start sending sub
            subghz_tx_stop_sub(app, true);
            app->chusen_sub = subrem_scene_remote_event_to_index(event.event);
            subrem_view_remote_set_state(app->subrem_remote_view, SubRemViewRemoteStateLoading);
            if(subghz_tx_start_sub(
                   app,
                   app->subs_preset[app->chusen_sub],
                   subrem_scene_remote_raw_callback_end_tx)) {
                subrem_view_remote_set_presed_btn(app->subrem_remote_view, app->chusen_sub);
                subrem_view_remote_set_state(
                    app->subrem_remote_view, SubRemViewRemoteStateSending);
                notification_message(app->notifications, &sequence_blink_start_magenta);
            } else {
                subrem_view_remote_set_state(app->subrem_remote_view, SubRemViewRemoteStateIdle);
            }
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteForcedStop) {
            subghz_tx_stop_sub(app, true);
            subrem_view_remote_set_presed_btn(app->subrem_remote_view, 0);
            subrem_view_remote_set_state(app->subrem_remote_view, SubRemViewRemoteStateIdle);

            notification_message(app->notifications, &sequence_blink_stop);
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteStop) {
            if(subghz_tx_stop_sub(app, false)) {
                subrem_view_remote_set_presed_btn(app->subrem_remote_view, 0);
                subrem_view_remote_set_state(app->subrem_remote_view, SubRemViewRemoteStateIdle);

                notification_message(app->notifications, &sequence_blink_stop);
            }
            return true;
        }
    }
    // } else if(event.type == SceneManagerEventTypeTick) {
    // }
    return false;
}

void subrem_scene_remote_on_exit(void* context) {
    SubGhzRemoteApp* app = context;

    subghz_tx_stop_sub(app, true);

    subrem_view_remote_set_presed_btn(app->subrem_remote_view, 0);
    subrem_view_remote_set_state(app->subrem_remote_view, SubRemViewRemoteStateIdle);

    notification_message(app->notifications, &sequence_blink_stop);

    // TODO: notifications and reset KL

    // keeloq_reset_mfname();
    // keeloq_reset_kl_type();
    // keeloq_reset_original_btn();
    // subghz_custom_btns_reset();
    // star_line_reset_mfname();
    // star_line_reset_kl_type();
}
