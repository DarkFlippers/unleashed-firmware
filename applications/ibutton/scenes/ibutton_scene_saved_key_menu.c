#include "../ibutton_i.h"

enum SubmenuIndex {
    SubmenuIndexEmulate,
    SubmenuIndexWrite,
    SubmenuIndexEdit,
    SubmenuIndexDelete,
    SubmenuIndexInfo,
};

void ibutton_scene_saved_key_menu_submenu_callback(void* context, uint32_t index) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, index);
}

void ibutton_scene_saved_key_menu_on_enter(void* context) {
    iButton* ibutton = context;
    Submenu* submenu = ibutton->submenu;

    submenu_add_item(
        submenu,
        "Emulate",
        SubmenuIndexEmulate,
        ibutton_scene_saved_key_menu_submenu_callback,
        ibutton);
    if(ibutton_key_get_type(ibutton->key) == iButtonKeyDS1990) {
        submenu_add_item(
            submenu,
            "Write",
            SubmenuIndexWrite,
            ibutton_scene_saved_key_menu_submenu_callback,
            ibutton);
    }
    submenu_add_item(
        submenu, "Edit", SubmenuIndexEdit, ibutton_scene_saved_key_menu_submenu_callback, ibutton);
    submenu_add_item(
        submenu,
        "Delete",
        SubmenuIndexDelete,
        ibutton_scene_saved_key_menu_submenu_callback,
        ibutton);
    submenu_add_item(
        submenu, "Info", SubmenuIndexInfo, ibutton_scene_saved_key_menu_submenu_callback, ibutton);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneSavedKeyMenu));

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewSubmenu);
}

bool ibutton_scene_saved_key_menu_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(
            ibutton->scene_manager, iButtonSceneSavedKeyMenu, event.event);
        consumed = true;
        if(event.event == SubmenuIndexEmulate) {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneEmulate);
        } else if(event.event == SubmenuIndexWrite) {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneWrite);
        } else if(event.event == SubmenuIndexEdit) {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneAddValue);
        } else if(event.event == SubmenuIndexDelete) {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneDeleteConfirm);
        } else if(event.event == SubmenuIndexInfo) {
            scene_manager_next_scene(ibutton->scene_manager, iButtonSceneInfo);
        }
    }

    return consumed;
}

void ibutton_scene_saved_key_menu_on_exit(void* context) {
    iButton* ibutton = context;
    submenu_reset(ibutton->submenu);
}
