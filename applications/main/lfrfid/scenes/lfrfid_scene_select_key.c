#include "../lfrfid_i.h"

void lfrfid_scene_select_key_on_enter(void* context) {
    LfRfid* app = context;

    if(lfrfid_load_key_from_file_select(app)) {
        scene_manager_next_scene(app->scene_manager, LfRfidSceneSavedKeyMenu);
    } else {
        scene_manager_previous_scene(app->scene_manager);
    }
}

bool lfrfid_scene_select_key_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

void lfrfid_scene_select_key_on_exit(void* context) {
    UNUSED(context);
}
