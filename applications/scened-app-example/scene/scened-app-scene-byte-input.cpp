#include "scened-app-scene-byte-input.h"

void ScenedAppSceneByteInput::on_enter(ScenedApp* app, bool need_restore) {
    ByteInputVM* byte_input = app->view_controller;
    auto callback = cbc::obtain_connector(this, &ScenedAppSceneByteInput::result_callback);

    byte_input->set_result_callback(callback, NULL, app, data, 4);
    byte_input->set_header_text("Enter the key");

    app->view_controller.switch_to<ByteInputVM>();
}

bool ScenedAppSceneByteInput::on_event(ScenedApp* app, ScenedApp::Event* event) {
    bool consumed = false;

    if(event->type == ScenedApp::EventType::ByteEditResult) {
        app->scene_controller.switch_to_previous_scene();
        consumed = true;
    }

    return consumed;
}

void ScenedAppSceneByteInput::on_exit(ScenedApp* app) {
    app->view_controller.get<ByteInputVM>()->clean();
}

void ScenedAppSceneByteInput::result_callback(void* context) {
    ScenedApp* app = static_cast<ScenedApp*>(context);
    ScenedApp::Event event;

    event.type = ScenedApp::EventType::ByteEditResult;

    app->view_controller.send_event(&event);
}
