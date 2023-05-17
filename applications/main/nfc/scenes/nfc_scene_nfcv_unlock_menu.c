#include "../nfc_i.h"
#include <dolphin/dolphin.h>

enum SubmenuIndex {
    SubmenuIndexNfcVUnlockMenuManual,
    SubmenuIndexNfcVUnlockMenuTonieBox,
};

void nfc_scene_nfcv_unlock_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_nfcv_unlock_menu_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneNfcVUnlockMenu);
    submenu_add_item(
        submenu,
        "Enter PWD Manually",
        SubmenuIndexNfcVUnlockMenuManual,
        nfc_scene_nfcv_unlock_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Auth As TonieBox",
        SubmenuIndexNfcVUnlockMenuTonieBox,
        nfc_scene_nfcv_unlock_menu_submenu_callback,
        nfc);
    submenu_set_selected_item(submenu, state);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_nfcv_unlock_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexNfcVUnlockMenuManual) {
            nfc->dev->dev_data.nfcv_data.auth_method = NfcVAuthMethodManual;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneNfcVKeyInput);
            consumed = true;
        } else if(event.event == SubmenuIndexNfcVUnlockMenuTonieBox) {
            nfc->dev->dev_data.nfcv_data.auth_method = NfcVAuthMethodTonieBox;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneNfcVUnlock);
            DOLPHIN_DEED(DolphinDeedNfcRead);
            consumed = true;
        }
        scene_manager_set_scene_state(nfc->scene_manager, NfcSceneNfcVUnlockMenu, event.event);
    }
    return consumed;
}

void nfc_scene_nfcv_unlock_menu_on_exit(void* context) {
    Nfc* nfc = context;

    submenu_reset(nfc->submenu);
}
