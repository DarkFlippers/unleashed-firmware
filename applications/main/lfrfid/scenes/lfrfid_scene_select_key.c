#include "../lfrfid_i.h"

void lfrfid_scene_select_key_on_enter(void* context) {
    LfRfid* app = context;

    if(lfrfid_select_key_file(app)) {
        // A T5577 dump is not a key this app can read: it is blocks of a chip, and the app that
        // reads them opens it. A handoff that happens stops this app, so nothing below it runs
        // and this scene is not returned to - the list comes back as a fresh launch.
        if(furi_string_end_withi(app->file_path, T5577_FILE_EXTENSION)) {
            if(lfrfid_run_t5577_app(app)) return;
            // The handoff said why it could not happen; go back the way a cancel does.
        } else if(lfrfid_load_key_data(app, app->file_path, true)) {
            scene_manager_next_scene(app->scene_manager, LfRfidSceneSavedKeyMenu);
            return;
        }
    }

    // Always select "Saved" menu item when returning from this scene
    scene_manager_set_scene_state(app->scene_manager, LfRfidSceneStart, LfRfidMenuIndexSaved);
    scene_manager_search_and_switch_to_previous_scene(app->scene_manager, LfRfidSceneStart);
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
