#include "../nfc_app_i.h"

enum SubmenuIndex {
    SubmenuIndexMfUlUnlockMenuReader,
    SubmenuIndexMfUlUnlockMenuAmeebo,
    SubmenuIndexMfUlUnlockMenuXiaomi,
    SubmenuIndexMfUlUnlockMenuManual,
};

void nfc_scene_mf_ultralight_unlock_menu_submenu_callback(void* context, uint32_t index) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_mf_ultralight_unlock_menu_on_enter(void* context) {
    NfcApp* nfc = context;
    Submenu* submenu = nfc->submenu;

    uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfUltralightUnlockMenu);
    if(nfc_device_get_protocol(nfc->nfc_device) == NfcProtocolMfUltralight) {
        const MfUltralightData* mfu_data =
            nfc_device_get_data(nfc->nfc_device, NfcProtocolMfUltralight);
        // Hide for MFU-C since it uses 3DES
        if(mfu_data->type != MfUltralightTypeMfulC) {
            submenu_add_item(
                submenu,
                "Unlock With Reader",
                SubmenuIndexMfUlUnlockMenuReader,
                nfc_scene_mf_ultralight_unlock_menu_submenu_callback,
                nfc);
        }
    }
    submenu_add_item(
        submenu,
        "Auth As Ameebo",
        SubmenuIndexMfUlUnlockMenuAmeebo,
        nfc_scene_mf_ultralight_unlock_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Auth As Xiaomi Air Purifier",
        SubmenuIndexMfUlUnlockMenuXiaomi,
        nfc_scene_mf_ultralight_unlock_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Enter Password Manually",
        SubmenuIndexMfUlUnlockMenuManual,
        nfc_scene_mf_ultralight_unlock_menu_submenu_callback,
        nfc);
    submenu_set_selected_item(submenu, state);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mf_ultralight_unlock_menu_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexMfUlUnlockMenuManual) {
            nfc->mf_ul_auth->type = MfUltralightAuthTypeManual;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightKeyInput);
            consumed = true;
        } else if(event.event == SubmenuIndexMfUlUnlockMenuAmeebo) {
            nfc->mf_ul_auth->type = MfUltralightAuthTypeAmiibo;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockWarn);
            consumed = true;
        } else if(event.event == SubmenuIndexMfUlUnlockMenuXiaomi) {
            nfc->mf_ul_auth->type = MfUltralightAuthTypeXiaomi;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockWarn);
            consumed = true;
        } else if(event.event == SubmenuIndexMfUlUnlockMenuReader) {
            nfc->mf_ul_auth->type = MfUltralightAuthTypeReader;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightCapturePass);
            consumed = true;
        }
        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneMfUltralightUnlockMenu, event.event);
    }
    return consumed;
}

void nfc_scene_mf_ultralight_unlock_menu_on_exit(void* context) {
    NfcApp* nfc = context;

    submenu_reset(nfc->submenu);
}
