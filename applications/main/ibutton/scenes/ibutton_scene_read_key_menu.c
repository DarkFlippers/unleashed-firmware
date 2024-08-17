#include "../ibutton_i.h"
#include <dolphin/dolphin.h>

typedef enum {
    SubmenuIndexSave,
    SubmenuIndexEmulate,
    SubmenuIndexViewData,
    SubmenuIndexWriteId,
    SubmenuIndexWriteCopy,
} SubmenuIndex;

void ibutton_scene_read_key_menu_submenu_callback(void* context, uint32_t index) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, index);
}

void ibutton_scene_read_key_menu_on_enter(void* context) {
    iButton* ibutton = context;
    Submenu* submenu = ibutton->submenu;

    const iButtonProtocolId protocol_id = ibutton_key_get_protocol_id(ibutton->key);
    const uint32_t features = ibutton_protocols_get_features(ibutton->protocols, protocol_id);

    submenu_add_item(
        submenu, "Save", SubmenuIndexSave, ibutton_scene_read_key_menu_submenu_callback, ibutton);
    submenu_add_item(
        submenu,
        "Emulate",
        SubmenuIndexEmulate,
        ibutton_scene_read_key_menu_submenu_callback,
        ibutton);

    if(features & iButtonProtocolFeatureWriteId) {
        submenu_add_item(
            submenu,
            "Write ID",
            SubmenuIndexWriteId,
            ibutton_scene_read_key_menu_submenu_callback,
            ibutton);
    }

    if(features & iButtonProtocolFeatureWriteCopy) {
        submenu_add_item(
            submenu,
            "Full Write on Same Type",
            SubmenuIndexWriteCopy,
            ibutton_scene_read_key_menu_submenu_callback,
            ibutton);
    }

    if(features & iButtonProtocolFeatureExtData) {
        submenu_add_item(
            submenu,
            "Data Info",
            SubmenuIndexViewData,
            ibutton_scene_read_key_menu_submenu_callback,
            ibutton);
    }

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(ibutton->scene_manager, iButtonSceneReadKeyMenu));
    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewSubmenu);
}

bool ibutton_scene_read_key_menu_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(scene_manager, iButtonSceneReadKeyMenu, event.event);
        consumed = true;

        if(event.event == SubmenuIndexSave) {
            scene_manager_next_scene(scene_manager, iButtonSceneSaveName);
        } else if(event.event == SubmenuIndexEmulate) {
            scene_manager_next_scene(scene_manager, iButtonSceneEmulate);
            dolphin_deed(DolphinDeedIbuttonEmulate);
        } else if(event.event == SubmenuIndexViewData) {
            scene_manager_next_scene(scene_manager, iButtonSceneViewData);
        } else if(event.event == SubmenuIndexWriteId) {
            ibutton->write_mode = iButtonWriteModeId;
            scene_manager_next_scene(scene_manager, iButtonSceneWrite);
        } else if(event.event == SubmenuIndexWriteCopy) {
            ibutton->write_mode = iButtonWriteModeCopy;
            scene_manager_next_scene(scene_manager, iButtonSceneWrite);
        }
    } else if(event.event == SceneManagerEventTypeBack) {
        scene_manager_set_scene_state(
            ibutton->scene_manager, iButtonSceneReadKeyMenu, SubmenuIndexSave);
        // Event is not consumed
    }

    return consumed;
}

void ibutton_scene_read_key_menu_on_exit(void* context) {
    iButton* ibutton = context;
    submenu_reset(ibutton->submenu);
}
