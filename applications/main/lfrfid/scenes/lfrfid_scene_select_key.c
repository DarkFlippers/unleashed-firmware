#include "../lfrfid_i.h"

void lfrfid_scene_select_key_on_enter(void* context) {
    LfRfid* app = context;

    if(lfrfid_load_key_from_file_select(app)) {
        scene_manager_next_scene(app->scene_manager, LfRfidSceneSavedKeyMenu);
    } else {
        // Always select "Saved" menu item when returning from this scene
        scene_manager_set_scene_state(app->scene_manager, LfRfidSceneStart, LfRfidMenuIndexSaved);
        scene_manager_search_and_switch_to_previous_scene(app->scene_manager, LfRfidSceneStart);
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
