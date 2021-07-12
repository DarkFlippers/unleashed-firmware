#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexRunApp,
    SubmenuIndexChooseScript,
    SubmenuIndexEmulate,
    SubmenuIndexSave,
};

void nfc_scene_card_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, index);
}

const void nfc_scene_card_menu_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu,
        "Run compatible app",
        SubmenuIndexRunApp,
        nfc_scene_card_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Additional reading scripts",
        SubmenuIndexChooseScript,
        nfc_scene_card_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu, "Emulate UID", SubmenuIndexEmulate, nfc_scene_card_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Name and save UID", SubmenuIndexSave, nfc_scene_card_menu_submenu_callback, nfc);
    submenu_set_selected_item(
        nfc->submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneCardMenu));

    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewMenu);
}

const bool nfc_scene_card_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexRunApp) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneCardMenu, SubmenuIndexRunApp);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneNotImplemented);
            return true;
        } else if(event.event == SubmenuIndexChooseScript) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneCardMenu, SubmenuIndexChooseScript);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneNotImplemented);
            return true;
        } else if(event.event == SubmenuIndexEmulate) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneCardMenu, SubmenuIndexEmulate);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneEmulateUid);
            return true;
        } else if(event.event == SubmenuIndexSave) {
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneCardMenu, SubmenuIndexSave);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveName);
            return true;
        }
    } else if(event.type == SceneManagerEventTypeNavigation) {
        return scene_manager_search_previous_scene(nfc->scene_manager, NfcSceneStart);
    }

    return false;
}

const void nfc_scene_card_menu_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    submenu_clean(nfc->submenu);
}
