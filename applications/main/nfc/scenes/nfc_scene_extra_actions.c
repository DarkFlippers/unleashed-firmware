#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexReadCardType,
    SubmenuIndexMfClassicKeys,
    SubmenuIndexMfUltralightUnlock,
};

void nfc_scene_extra_actions_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_extra_actions_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu,
        "Read Specific Card Type",
        SubmenuIndexReadCardType,
        nfc_scene_extra_actions_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Mifare Classic Keys",
        SubmenuIndexMfClassicKeys,
        nfc_scene_extra_actions_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Unlock NTAG/Ultralight",
        SubmenuIndexMfUltralightUnlock,
        nfc_scene_extra_actions_submenu_callback,
        nfc);
    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneExtraActions));
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_extra_actions_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexMfClassicKeys) {
            if(mf_classic_dict_check_presence(MfClassicDictTypeSystem)) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicKeys);
            } else {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneDictNotFound);
            }
            consumed = true;
        } else if(event.event == SubmenuIndexMfUltralightUnlock) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockMenu);
            consumed = true;
        } else if(event.event == SubmenuIndexReadCardType) {
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneReadCardType, 0);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneReadCardType);
            consumed = true;
        }
        scene_manager_set_scene_state(nfc->scene_manager, NfcSceneExtraActions, event.event);
    }

    return consumed;
}

void nfc_scene_extra_actions_on_exit(void* context) {
    Nfc* nfc = context;

    submenu_reset(nfc->submenu);
}
