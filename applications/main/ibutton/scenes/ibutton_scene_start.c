#include "../ibutton_i.h"
#include "ibutton/scenes/ibutton_scene.h"

enum SubmenuIndex {
    SubmenuIndexRead,
    SubmenuIndexSaved,
    SubmenuIndexAdd,
};

void ibutton_scene_start_submenu_callback(void* context, uint32_t index) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, index);
}

void ibutton_scene_start_on_enter(void* context) {
    iButton* ibutton = context;
    Submenu* submenu = ibutton->submenu;

    submenu_add_item(
        submenu, "Read", SubmenuIndexRead, ibutton_scene_start_submenu_callback, ibutton);
    submenu_add_item(
        submenu, "Saved", SubmenuIndexSaved, ibutton_scene_start_submenu_callback, ibutton);
    submenu_add_item(
        submenu, "Add Manually", SubmenuIndexAdd, ibutton_scene_start_submenu_callback, ibutton);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneStart));

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewSubmenu);
}

bool ibutton_scene_start_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(ibutton->scene_manager, iButtonSceneStart, event.event);
        consumed = true;
        if(event.event == SubmenuIndexRead) {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneRead);
        } else if(event.event == SubmenuIndexSaved) {
            string_set_str(ibutton->file_path, IBUTTON_APP_FOLDER);
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneSelectKey);
        } else if(event.event == SubmenuIndexAdd) {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneAddType);
        }
    }

    return consumed;
}

void ibutton_scene_start_on_exit(void* context) {
    iButton* ibutton = context;
    submenu_reset(ibutton->submenu);
}
