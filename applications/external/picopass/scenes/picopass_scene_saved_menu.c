#include "../picopass_i.h"

enum SubmenuIndex {
    SubmenuIndexDelete,
    SubmenuIndexInfo,
    SubmenuIndexWrite,
};

void picopass_scene_saved_menu_submenu_callback(void* context, uint32_t index) {
    Picopass* picopass = context;

    view_dispatcher_send_custom_event(picopass->view_dispatcher, index);
}

void picopass_scene_saved_menu_on_enter(void* context) {
    Picopass* picopass = context;
    Submenu* submenu = picopass->submenu;

    submenu_add_item(
        submenu,
        "Delete",
        SubmenuIndexDelete,
        picopass_scene_saved_menu_submenu_callback,
        picopass);
    submenu_add_item(
        submenu, "Info", SubmenuIndexInfo, picopass_scene_saved_menu_submenu_callback, picopass);
    submenu_add_item(
        submenu, "Write", SubmenuIndexWrite, picopass_scene_saved_menu_submenu_callback, picopass);

    submenu_set_selected_item(
        picopass->submenu,
        scene_manager_get_scene_state(picopass->scene_manager, PicopassSceneSavedMenu));

    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewMenu);
}

bool picopass_scene_saved_menu_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(
            picopass->scene_manager, PicopassSceneSavedMenu, event.event);

        if(event.event == SubmenuIndexDelete) {
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneDelete);
            consumed = true;
        } else if(event.event == SubmenuIndexInfo) {
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneDeviceInfo);
            consumed = true;
        } else if(event.event == SubmenuIndexWrite) {
            scene_manager_next_scene(picopass->scene_manager, PicopassSceneWriteCard);
            consumed = true;
        }
    }

    return consumed;
}

void picopass_scene_saved_menu_on_exit(void* context) {
    Picopass* picopass = context;

    submenu_reset(picopass->submenu);
}
