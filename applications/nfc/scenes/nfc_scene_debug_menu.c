#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexDetect,
    SubmenuIndexEmulate,
    SubmenuIndexReadEmv,
    SubmenuIndexReadMifareUl,
};

void nfc_scene_debug_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, index);
}

const void nfc_scene_debug_menu_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu, "Detect", SubmenuIndexDetect, nfc_scene_debug_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Emulate", SubmenuIndexEmulate, nfc_scene_debug_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Read EMV", SubmenuIndexReadEmv, nfc_scene_debug_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu,
        "Read Mifare Ultralight",
        SubmenuIndexReadMifareUl,
        nfc_scene_debug_menu_submenu_callback,
        nfc);
    submenu_set_selected_item(
        nfc->submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneDebugMenu));

    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewMenu);
}

const bool nfc_scene_debug_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexDetect) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneDebugMenu, SubmenuIndexDetect);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneDebugDetect);
            return true;
        } else if(event.event == SubmenuIndexEmulate) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneDebugMenu, SubmenuIndexEmulate);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneDebugEmulate);
            return true;
        } else if(event.event == SubmenuIndexReadEmv) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneDebugMenu, SubmenuIndexReadEmv);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneDebugReadEmv);
            return true;
        } else if(event.event == SubmenuIndexReadMifareUl) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneDebugMenu, SubmenuIndexReadMifareUl);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneDebugReadMifareUl);
            return true;
        }
    }

    return false;
}

const void nfc_scene_debug_menu_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    submenu_clean(nfc->submenu);
}
