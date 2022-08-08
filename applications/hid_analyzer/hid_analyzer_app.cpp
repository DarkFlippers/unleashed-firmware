#include "hid_analyzer_app.h"
#include "scene/hid_analyzer_app_scene_read.h"
#include "scene/hid_analyzer_app_scene_read_success.h"

HIDApp::HIDApp()
    : scene_controller{this}
    , notification{"notification"}
    , storage{"storage"}
    , dialogs{"dialogs"}
    , text_store(40) {
}

HIDApp::~HIDApp() {
}

void HIDApp::run(void* _args) {
    UNUSED(_args);

    scene_controller.add_scene(SceneType::Read, new HIDAppSceneRead());
    scene_controller.add_scene(SceneType::ReadSuccess, new HIDAppSceneReadSuccess());
    scene_controller.process(100, SceneType::Read);
}