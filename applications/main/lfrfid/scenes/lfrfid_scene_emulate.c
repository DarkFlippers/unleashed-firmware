#include "../lfrfid_i.h"

#define LFRFID_EMULATION_TIME_MAX_MS (5 * 60 * 1000)

FuriTimer* timer_auto_exit;

void lfrfid_scene_emulate_popup_callback(void* context) {
    LfRfid* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, LfRfidEventEmulationTimeExpired);
}

void lfrfid_scene_emulate_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    popup_set_header(popup, "Emulating", 89, 30, AlignCenter, AlignTop);
    if(!furi_string_empty(app->file_name)) {
        popup_set_text(popup, furi_string_get_cstr(app->file_name), 89, 43, AlignCenter, AlignTop);
    } else {
        popup_set_text(
            popup,
            protocol_dict_get_name(app->dict, app->protocol_id),
            89,
            43,
            AlignCenter,
            AlignTop);
    }
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinSend_97x61);

    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_emulate_start(app->lfworker, (LFRFIDProtocol)app->protocol_id);
    notification_message(app->notifications, &sequence_blink_start_magenta);

    timer_auto_exit =
        furi_timer_alloc(lfrfid_scene_emulate_popup_callback, FuriTimerTypeOnce, app);

    if(!furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug))
        furi_timer_start(timer_auto_exit, LFRFID_EMULATION_TIME_MAX_MS);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

bool lfrfid_scene_emulate_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventEmulationTimeExpired) {
            if(!scene_manager_previous_scene(app->scene_manager)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            } else {
                scene_manager_previous_scene(app->scene_manager);
            }
            consumed = true;
        }
    }

    return consumed;
}

void lfrfid_scene_emulate_on_exit(void* context) {
    LfRfid* app = context;

    furi_timer_stop(timer_auto_exit);
    furi_timer_free(timer_auto_exit);

    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);
}
