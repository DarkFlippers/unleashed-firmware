#include "lfrfid_debug_app.h"
#include "scene/lfrfid_debug_app_scene_start.h"
#include "scene/lfrfid_debug_app_scene_tune.h"

LfRfidDebugApp::LfRfidDebugApp()
    : scene_controller{this} {
}

LfRfidDebugApp::~LfRfidDebugApp() {
}

void LfRfidDebugApp::run() {
    view_controller.attach_to_gui(ViewDispatcherTypeFullscreen);
    scene_controller.add_scene(SceneType::Start, new LfRfidDebugAppSceneStart());
    scene_controller.add_scene(SceneType::TuneScene, new LfRfidDebugAppSceneTune());
    scene_controller.process(100);
}
