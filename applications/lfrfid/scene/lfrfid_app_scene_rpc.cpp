#include "lfrfid_app_scene_rpc.h"
#include <core/common_defines.h>
#include <dolphin/dolphin.h>

void LfRfidAppSceneRpc::on_enter(LfRfidApp* app, bool /* need_restore */) {
    auto popup = app->view_controller.get<PopupVM>();

    popup->set_header("RPC Mode", 64, 30, AlignCenter, AlignTop);

    app->view_controller.switch_to<PopupVM>();

    notification_message(app->notification, &sequence_display_backlight_on);
}

bool LfRfidAppSceneRpc::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    UNUSED(app);
    UNUSED(event);
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Exit) {
        consumed = true;
        LfRfidApp::Event view_event;
        view_event.type = LfRfidApp::EventType::Back;
        app->view_controller.send_event(&view_event);
    } else if(event->type == LfRfidApp::EventType::EmulateStart) {
        consumed = true;
        emulating = true;
    }
    return consumed;
}

void LfRfidAppSceneRpc::on_exit(LfRfidApp* app) {
    if(emulating) {
        app->worker.stop_emulate();
    }
    app->view_controller.get<PopupVM>()->clean();
}
