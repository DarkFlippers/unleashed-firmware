#include "../subghz_remote_app_i.h"
#include "../views/remote.h"

#include <lib/subghz/protocols/raw.h>

#define TAG "SubRemScenRemote"

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

void subrem_scene_remote_on_enter(void* context) {
    SubGhzRemoteApp* app = context;

    subrem_view_remote_update_data_labels(app->subrem_remote_view, app->map_preset->subs_preset);
    subrem_view_remote_set_radio(
        app->subrem_remote_view,
        subghz_txrx_radio_device_get(app->txrx) != SubGhzRadioDeviceTypeInternal);

    subrem_view_remote_set_callback(app->subrem_remote_view, subrem_scene_remote_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDRemote);
}

bool subrem_scene_remote_on_event(void* context, SceneManagerEvent event) {
    SubGhzRemoteApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubRemCustomEventViewRemoteBack) {
            if(!scene_manager_previous_scene(app->scene_manager)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }
            return true;
        } else if(
            event.event == SubRemCustomEventViewRemoteStartUP ||
            event.event == SubRemCustomEventViewRemoteStartDOWN ||
            event.event == SubRemCustomEventViewRemoteStartLEFT ||
            event.event == SubRemCustomEventViewRemoteStartRIGHT ||
            event.event == SubRemCustomEventViewRemoteStartOK) {
            // Start sending sub
            subrem_tx_stop_sub(app, true);

            uint8_t chusen_sub = subrem_scene_remote_event_to_index(event.event);
            app->chusen_sub = chusen_sub;

            subrem_view_remote_set_state(
                app->subrem_remote_view, SubRemViewRemoteStateLoading, chusen_sub);

            if(subrem_tx_start_sub(app, app->map_preset->subs_preset[chusen_sub])) {
                if(app->map_preset->subs_preset[chusen_sub]->type == SubGhzProtocolTypeRAW) {
                    subghz_txrx_set_raw_file_encoder_worker_callback_end(
                        app->txrx, subrem_scene_remote_raw_callback_end_tx, app);
                }
                subrem_view_remote_set_state(
                    app->subrem_remote_view, SubRemViewRemoteStateSending, chusen_sub);
                notification_message(app->notifications, &sequence_blink_start_magenta);
            } else {
                subrem_view_remote_set_state(
                    app->subrem_remote_view, SubRemViewRemoteStateIdle, 0);
                notification_message(app->notifications, &sequence_blink_red_100);
            }
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteForcedStop) {
            subrem_tx_stop_sub(app, true);
            subrem_view_remote_set_state(app->subrem_remote_view, SubRemViewRemoteStateIdle, 0);

            notification_message(app->notifications, &sequence_blink_stop);
            return true;
        } else if(event.event == SubRemCustomEventViewRemoteStop) {
            if(subrem_tx_stop_sub(app, false)) {
                subrem_view_remote_set_state(
                    app->subrem_remote_view, SubRemViewRemoteStateIdle, 0);

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

    subrem_tx_stop_sub(app, true);

    subrem_view_remote_set_state(app->subrem_remote_view, SubRemViewRemoteStateIdle, 0);

    notification_message(app->notifications, &sequence_blink_stop);
}
