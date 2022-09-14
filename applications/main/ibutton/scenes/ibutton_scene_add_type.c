#include "../ibutton_i.h"
#include "m-string.h"

enum SubmenuIndex {
    SubmenuIndexCyfral,
    SubmenuIndexDallas,
    SubmenuIndexMetakom,
};

void ibutton_scene_add_type_submenu_callback(void* context, uint32_t index) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, index);
}

void ibutton_scene_add_type_on_enter(void* context) {
    iButton* ibutton = context;
    Submenu* submenu = ibutton->submenu;

    submenu_add_item(
        submenu, "Cyfral", SubmenuIndexCyfral, ibutton_scene_add_type_submenu_callback, ibutton);
    submenu_add_item(
        submenu, "Dallas", SubmenuIndexDallas, ibutton_scene_add_type_submenu_callback, ibutton);
    submenu_add_item(
        submenu, "Metakom", SubmenuIndexMetakom, ibutton_scene_add_type_submenu_callback, ibutton);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneAddType));

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewSubmenu);
}

bool ibutton_scene_add_type_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    iButtonKey* key = ibutton->key;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(ibutton->scene_manager, iButtonSceneAddType, event.event);
        consumed = true;
        if(event.event == SubmenuIndexCyfral) {
            ibutton_key_set_type(key, iButtonKeyCyfral);
        } else if(event.event == SubmenuIndexDallas) {
            ibutton_key_set_type(key, iButtonKeyDS1990);
        } else if(event.event == SubmenuIndexMetakom) {
            ibutton_key_set_type(key, iButtonKeyMetakom);
        } else {
            furi_crash("Unknown key type");
        }

        string_set_str(ibutton->file_path, IBUTTON_APP_FOLDER);
        ibutton_key_clear_data(key);
        scene_manager_next_scene(ibutton->scene_manager, iButtonSceneAddValue);
    }

    return consumed;
}

void ibutton_scene_add_type_on_exit(void* context) {
    iButton* ibutton = context;
    submenu_reset(ibutton->submenu);
}
