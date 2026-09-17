#include "../ibutton_i.h"

enum SubmenuIndex {
    SubmenuIndexWriteTargets,
};

static void ibutton_scene_settings_submenu_callback(void* context, uint32_t index) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, index);
}

void ibutton_scene_settings_on_enter(void* context) {
    iButton* ibutton = context;
    Submenu* submenu = ibutton->submenu;

    submenu_add_item(
        submenu,
        "Write Blanks",
        SubmenuIndexWriteTargets,
        ibutton_scene_settings_submenu_callback,
        ibutton);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneSettings));

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewSubmenu);
}

bool ibutton_scene_settings_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(ibutton->scene_manager, iButtonSceneSettings, event.event);
        consumed = true;

        if(event.event == SubmenuIndexWriteTargets) {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneSettingsWriteTargets);
        }

    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_set_scene_state(ibutton->scene_manager, iButtonSceneSettings, 0);
    }

    return consumed;
}

void ibutton_scene_settings_on_exit(void* context) {
    iButton* ibutton = context;
    submenu_reset(ibutton->submenu);
}
