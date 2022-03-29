#include "ibutton_scene_select_key.h"
#include "../ibutton_app.h"

void iButtonSceneSelectKey::on_enter(iButtonApp* app) {
    // Process file_select return
    if(app->load_key()) {
        app->switch_to_next_scene(iButtonApp::Scene::SceneSavedKeyMenu);
    } else {
        app->switch_to_previous_scene();
    }
}

bool iButtonSceneSelectKey::on_event(iButtonApp* app, iButtonEvent* event) {
    return false;
}

void iButtonSceneSelectKey::on_exit(iButtonApp* app) {
}
