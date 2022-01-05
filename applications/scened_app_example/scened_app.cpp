#include "scened_app.h"
#include "scene/scened_app_scene_start.h"
#include "scene/scened_app_scene_byte_input.h"

ScenedApp::ScenedApp()
    : scene_controller{this}
    , text_store{128}
    , notification{"notification"} {
}

ScenedApp::~ScenedApp() {
}

void ScenedApp::run() {
    scene_controller.add_scene(SceneType::Start, new ScenedAppSceneStart());
    scene_controller.add_scene(SceneType::ByteInputScene, new ScenedAppSceneByteInput());

    notification_message(notification, &sequence_blink_green_10);
    scene_controller.process(100);
}