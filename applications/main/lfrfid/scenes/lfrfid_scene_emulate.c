#include "../lfrfid_i.h"
#include <dolphin/dolphin.h>

void lfrfid_scene_emulate_on_enter(void* context) {
    LfRfid* app = context;
    Popup* popup = app->popup;

    DOLPHIN_DEED(DolphinDeedRfidEmulate);

    popup_set_header(popup, "Emulating", 89, 30, AlignCenter, AlignTop);
    if(!string_empty_p(app->file_name)) {
        popup_set_text(popup, string_get_cstr(app->file_name), 89, 43, AlignCenter, AlignTop);
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

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewPopup);
}

bool lfrfid_scene_emulate_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

void lfrfid_scene_emulate_on_exit(void* context) {
    LfRfid* app = context;
    notification_message(app->notifications, &sequence_blink_stop);
    popup_reset(app->popup);
    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);
}
