#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexSave,
    SubmenuIndexEmulate,
};

void nfc_scene_mifare_ul_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

const void nfc_scene_mifare_ul_menu_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu, "Name and save", SubmenuIndexSave, nfc_scene_mifare_ul_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Emulate", SubmenuIndexEmulate, nfc_scene_mifare_ul_menu_submenu_callback, nfc);
    submenu_set_selected_item(
        nfc->submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMifareUlMenu));

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

const bool nfc_scene_mifare_ul_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexSave) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMifareUlMenu, SubmenuIndexSave);
            nfc->dev.format = NfcDeviceSaveFormatMifareUl;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveName);
            return true;
        } else if(event.event == SubmenuIndexEmulate) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMifareUlMenu, SubmenuIndexEmulate);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneEmulateMifareUl);
            return true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        return scene_manager_search_and_switch_to_previous_scene(
            nfc->scene_manager, NfcSceneStart);
    }

    return false;
}

const void nfc_scene_mifare_ul_menu_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    submenu_clean(nfc->submenu);
}
