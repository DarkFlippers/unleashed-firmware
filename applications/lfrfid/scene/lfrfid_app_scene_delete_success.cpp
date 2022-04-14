#include "lfrfid_app_scene_delete_success.h"

void LfRfidAppSceneDeleteSuccess::on_enter(LfRfidApp* app, bool need_restore) {
    auto popup = app->view_controller.get<PopupVM>();

    popup->set_icon(0, 2, &I_DolphinMafia_115x62);
    popup->set_header("Deleted", 83, 19, AlignLeft, AlignBottom);
    popup->set_context(app);
    popup->set_callback(LfRfidAppSceneDeleteSuccess::timeout_callback);
    popup->set_timeout(1500);
    popup->enable_timeout();

    app->view_controller.switch_to<PopupVM>();
}

bool LfRfidAppSceneDeleteSuccess::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Back) {
        app->scene_controller.search_and_switch_to_previous_scene(
            {LfRfidApp::SceneType::SelectKey});

        consumed = true;
    }

    return consumed;
}

void LfRfidAppSceneDeleteSuccess::on_exit(LfRfidApp* app) {
    app->view_controller.get<PopupVM>()->clean();
}

void LfRfidAppSceneDeleteSuccess::timeout_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Back;
    app->view_controller.send_event(&event);
}