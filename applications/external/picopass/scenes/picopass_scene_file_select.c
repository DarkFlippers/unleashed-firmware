#include "../picopass_i.h"
#include "../picopass_device.h"

void picopass_scene_file_select_on_enter(void* context) {
    Picopass* picopass = context;
    // Process file_select return
    picopass_device_set_loading_callback(picopass->dev, picopass_show_loading_popup, picopass);
    if(picopass_file_select(picopass->dev)) {
        scene_manager_next_scene(picopass->scene_manager, PicopassSceneSavedMenu);
    } else {
        scene_manager_search_and_switch_to_previous_scene(
            picopass->scene_manager, PicopassSceneStart);
    }
    picopass_device_set_loading_callback(picopass->dev, NULL, picopass);
}

bool picopass_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void picopass_scene_file_select_on_exit(void* context) {
    UNUSED(context);
}
