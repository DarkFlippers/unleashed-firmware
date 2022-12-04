#include "../namechanger.h"

enum SubmenuIndex {
    SubmenuIndexChange,
    SubmenuIndexRevert,
};

void namechanger_scene_start_submenu_callback(void* context, uint32_t index) {
    NameChanger* namechanger = context;
    view_dispatcher_send_custom_event(namechanger->view_dispatcher, index);
}

void namechanger_scene_start_on_enter(void* context) {
    NameChanger* namechanger = context;
    Submenu* submenu = namechanger->submenu;

    submenu_add_item(
        submenu,
        "Change",
        SubmenuIndexChange,
        namechanger_scene_start_submenu_callback,
        namechanger);

    submenu_add_item(
        submenu,
        "Revert",
        SubmenuIndexRevert,
        namechanger_scene_start_submenu_callback,
        namechanger);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(namechanger->scene_manager, NameChangerSceneStart));

    view_dispatcher_switch_to_view(namechanger->view_dispatcher, NameChangerViewSubmenu);
}

bool namechanger_scene_start_on_event(void* context, SceneManagerEvent event) {
    NameChanger* namechanger = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(
            namechanger->scene_manager, NameChangerSceneStart, event.event);
        consumed = true;
        if(event.event == SubmenuIndexChange) {
            scene_manager_next_scene(namechanger->scene_manager, NameChangerSceneChange);
        }
        if(event.event == SubmenuIndexRevert) {
            scene_manager_next_scene(namechanger->scene_manager, NameChangerSceneRevert);
        }
    }
    return consumed;
}

void namechanger_scene_start_on_exit(void* context) {
    NameChanger* namechanger = context;
    submenu_reset(namechanger->submenu);
}
