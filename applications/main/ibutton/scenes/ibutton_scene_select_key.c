#include "../ibutton_i.h"

void ibutton_scene_select_key_on_enter(void* context) {
    iButton* ibutton = context;

    if(ibutton_select_and_load_key(ibutton)) {
        scene_manager_next_scene(ibutton->scene_manager, iButtonSceneSavedKeyMenu);
    } else {
        scene_manager_search_and_switch_to_previous_scene(
            ibutton->scene_manager, iButtonSceneStart);
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
