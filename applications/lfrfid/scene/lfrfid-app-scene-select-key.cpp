#include "lfrfid-app-scene-select-key.h"

void LfRfidAppSceneSelectKey::on_enter(LfRfidApp* app, bool need_restore) {
    if(app->load_key_from_file_select(need_restore)) {
        app->scene_controller.switch_to_next_scene(LfRfidApp::SceneType::SavedKeyMenu);
    } else {
        app->scene_controller.switch_to_previous_scene();
    }
}

bool LfRfidAppSceneSelectKey::on_event(LfRfidApp* app, LfRfidApp::Event* event) {
    bool consumed = false;

    return consumed;
}

void LfRfidAppSceneSelectKey::on_exit(LfRfidApp* app) {
}