#include "lfrfid_app_scene_emulate.h"
#include <core/common_defines.h>
#include <dolphin/dolphin.h>

void LfRfidAppSceneEmulate::on_enter(LfRfidApp* app, bool /* need_restore */) {
    DOLPHIN_DEED(DolphinDeedRfidEmulate);
    auto popup = app->view_controller.get<PopupVM>();

    popup->set_header("Emulating", 89, 30, AlignCenter, AlignTop);
    if(string_size(app->file_name)) {
        popup->set_text(string_get_cstr(app->file_name), 89, 43, AlignCenter, AlignTop);
    } else {
        popup->set_text(
            protocol_dict_get_name(app->dict, app->protocol_id), 89, 43, AlignCenter, AlignTop);
    }
    popup->set_icon(0, 3, &I_RFIDDolphinSend_97x61);

    app->view_controller.switch_to<PopupVM>();
    lfrfid_worker_start_thread(app->lfworker);
    lfrfid_worker_emulate_start(app->lfworker, (LFRFIDProtocol)app->protocol_id);
    notification_message(app->notification, &sequence_blink_start_magenta);
}

bool LfRfidAppSceneEmulate::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    UNUSED(app);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

void LfRfidAppSceneEmulate::on_exit(LfRfidApp* app) {
    app->view_controller.get<PopupVM>()->clean();
    lfrfid_worker_stop(app->lfworker);
    lfrfid_worker_stop_thread(app->lfworker);
    notification_message(app->notification, &sequence_blink_stop);
}
