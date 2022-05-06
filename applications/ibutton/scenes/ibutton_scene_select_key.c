#include "../ibutton_i.h"

void ibutton_scene_select_key_on_enter(void* context) {
    iButton* ibutton = context;

    if(!ibutton_file_select(ibutton)) {
        scene_manager_search_and_switch_to_previous_scene(
            ibutton->scene_manager, iButtonSceneStart);
    } else {
        scene_manager_next_scene(ibutton->scene_manager, iButtonSceneSavedKeyMenu);
    }
}

bool ibutton_scene_select_key_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void ibutton_scene_select_key_on_exit(void* context) {
    UNUSED(context);
}
