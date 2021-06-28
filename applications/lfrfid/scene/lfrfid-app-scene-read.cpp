#include "lfrfid-app-scene-read.h"

void LfRfidAppSceneRead::on_enter(LfRfidApp* app, bool need_restore) {
    auto popup = app->view_controller.get<PopupVM>();

    popup->set_header("Reading\nLF RFID", 70, 34, AlignLeft, AlignTop);
    popup->set_icon(0, 4, I_RFIDDolphinReceive_98x60);

    app->view_controller.switch_to<PopupVM>();
    app->worker.start_read();
}

bool LfRfidAppSceneRead::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    if(event->type == LfRfidApp::EventType::Tick) {
        if(app->worker.read()) {
            notification_message(app->notification, &sequence_success);
            app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::ReadSuccess);
        } else {
            notification_message(app->notification, &sequence_blink_red_10);
        }
    }

    return consumed;
}

void LfRfidAppSceneRead::on_exit(LfRfidApp* app) {
    app->view_controller.get<PopupVM>()->clean();
    app->worker.stop_read();
}
