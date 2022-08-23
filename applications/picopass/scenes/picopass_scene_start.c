#include "../picopass_i.h"
enum SubmenuIndex {
    SubmenuIndexRead,
    SubmenuIndexRunScript,
    SubmenuIndexSaved,
    SubmenuIndexAddManualy,
    SubmenuIndexDebug,
};

void picopass_scene_start_submenu_callback(void* context, uint32_t index) {
    Picopass* picopass = context;
    view_dispatcher_send_custom_event(picopass->view_dispatcher, index);
}
void picopass_scene_start_on_enter(void* context) {
    Picopass* picopass = context;

    Submenu* submenu = picopass->submenu;
    submenu_add_item(
        submenu, "Read Card", SubmenuIndexRead, picopass_scene_start_submenu_callback, picopass);
    submenu_add_item(
        submenu, "Saved", SubmenuIndexSaved, picopass_scene_start_submenu_callback, picopass);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(picopass->scene_manager, PicopassSceneStart));
    picopass_device_clear(picopass->dev);
    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewMenu);
}

bool picopass_scene_start_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexRead) {
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneReadCard);
            consumed = true;
        } else if(event.event == SubmenuIndexSaved) {
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneFileSelect);
            consumed = true;
        }
        scene_manager_set_scene_state(picopass->scene_manager, PicopassSceneStart, event.event);
    }

    return consumed;
}

void picopass_scene_start_on_exit(void* context) {
    Picopass* picopass = context;
    submenu_reset(picopass->submenu);
}
