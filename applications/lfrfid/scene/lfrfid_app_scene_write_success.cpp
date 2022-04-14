#include "lfrfid_app_scene_write_success.h"

void LfRfidAppSceneWriteSuccess::on_enter(LfRfidApp* app, bool need_restore) {
    auto popup = app->view_controller.get<PopupVM>();
    popup->set_header("Successfully\nwritten!", 94, 3, AlignCenter, AlignTop);
    popup->set_icon(0, 6, &I_RFIDDolphinSuccess_108x57);
    popup->set_context(app);
    popup->set_callback(LfRfidAppSceneWriteSuccess::timeout_callback);
    popup->set_timeout(1500);
    popup->enable_timeout();

    app->view_controller.switch_to<PopupVM>();
    notification_message_block(app->notification, &sequence_set_green_255);
}

bool LfRfidAppSceneWriteSuccess::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Back) {
        app->scene_controller.search_and_switch_to_previous_scene(
            {LfRfidApp::SceneType::ReadKeyMenu, LfRfidApp::SceneType::SelectKey});
        consumed = true;
    }

    return consumed;
}

void LfRfidAppSceneWriteSuccess::on_exit(LfRfidApp* app) {
    notification_message_block(app->notification, &sequence_reset_green);
    app->view_controller.get<PopupVM>()->clean();
}

void LfRfidAppSceneWriteSuccess::timeout_callback(void* context) {
    LfRfidApp* app = static_cast<LfRfidApp*>(context);
    LfRfidApp::Event event;
    event.type = LfRfidApp::EventType::Back;
    app->view_controller.send_event(&event);
}
