#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexMfUlUnlockMenuManual,
    SubmenuIndexMfUlUnlockMenuAmeebo,
    SubmenuIndexMfUlUnlockMenuXiaomi,
};

void nfc_scene_mf_ultralight_unlock_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_mf_ultralight_unlock_menu_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfUltralightUnlockMenu);
    submenu_add_item(
        submenu,
        "Enter Password Manually",
        SubmenuIndexMfUlUnlockMenuManual,
        nfc_scene_mf_ultralight_unlock_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Auth As Ameebo",
        SubmenuIndexMfUlUnlockMenuAmeebo,
        nfc_scene_mf_ultralight_unlock_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Auth As Xiaomi",
        SubmenuIndexMfUlUnlockMenuXiaomi,
        nfc_scene_mf_ultralight_unlock_menu_submenu_callback,
        nfc);
    submenu_set_selected_item(submenu, state);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mf_ultralight_unlock_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexMfUlUnlockMenuManual) {
            nfc->dev->dev_data.mf_ul_data.auth_method = MfUltralightAuthMethodManual;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightKeyInput);
            consumed = true;
        } else if(event.event == SubmenuIndexMfUlUnlockMenuAmeebo) {
            nfc->dev->dev_data.mf_ul_data.auth_method = MfUltralightAuthMethodAmeebo;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockWarn);
            consumed = true;
        } else if(event.event == SubmenuIndexMfUlUnlockMenuXiaomi) {
            nfc->dev->dev_data.mf_ul_data.auth_method = MfUltralightAuthMethodXiaomi;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockWarn);
            consumed = true;
        }
        scene_manager_set_scene_state(nfc->scene_manager, NfcSceneExtraActions, event.event);
    }
    return consumed;
}

void nfc_scene_mf_ultralight_unlock_menu_on_exit(void* context) {
    Nfc* nfc = context;

    submenu_reset(nfc->submenu);
}
