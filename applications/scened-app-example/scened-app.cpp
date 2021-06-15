#include "scened-app.h"
#include "scene/scened-app-scene-start.h"
#include "scene/scened-app-scene-byte-input.h"

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