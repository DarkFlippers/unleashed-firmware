#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexBankCard,
    SubmenuIndexMifareUltralight,
};

void nfc_scene_scripts_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_scripts_menu_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu,
        "Read bank card",
        SubmenuIndexBankCard,
        nfc_scene_scripts_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Read Mifare Ultral/Ntag",
        SubmenuIndexMifareUltralight,
        nfc_scene_scripts_menu_submenu_callback,
        nfc);
    submenu_set_selected_item(
        nfc->submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneScriptsMenu));
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_scripts_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexBankCard) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneScriptsMenu, SubmenuIndexBankCard);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneReadEmvApp);
            return true;
        } else if(event.event == SubmenuIndexMifareUltralight) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneScriptsMenu, SubmenuIndexMifareUltralight);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneReadMifareUl);
            return true;
        }
    }

    return false;
}

void nfc_scene_scripts_menu_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    submenu_reset(nfc->submenu);
}
