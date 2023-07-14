#include "../subghz_remote_app_i.h"

void subrem_scene_open_map_file_on_enter(void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;

    SubRemLoadMapState load_state = subrem_load_from_file(app);
    uint32_t start_scene_state =
        scene_manager_get_scene_state(app->scene_manager, SubRemSceneStart);

    if(load_state == SubRemLoadMapStateBack) {
        scene_manager_previous_scene(app->scene_manager);
    } else if(start_scene_state == SubmenuIndexSubRemEditMapFile) {
        scene_manager_set_scene_state(app->scene_manager, SubRemSceneEditMenu, SubRemSubKeyNameUp);
        scene_manager_next_scene(app->scene_manager, SubRemSceneEditMenu);
    } else if(start_scene_state == SubmenuIndexSubRemOpenMapFile) {
        scene_manager_next_scene(app->scene_manager, SubRemSceneRemote);
    }
}

bool subrem_scene_open_map_file_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void subrem_scene_open_map_file_on_exit(void* context) {
    UNUSED(context);
}
