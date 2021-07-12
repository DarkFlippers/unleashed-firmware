#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexNFCA4,
    SubmenuIndexNFCA7,
};

void nfc_scene_set_type_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, index);
}

const void nfc_scene_set_type_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu, "NFC-A 7-bytes UID", SubmenuIndexNFCA7, nfc_scene_set_type_submenu_callback, nfc);
    submenu_add_item(
        submenu, "NFC-A 4-bytes UID", SubmenuIndexNFCA4, nfc_scene_set_type_submenu_callback, nfc);
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewMenu);
}

const bool nfc_scene_set_type_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexNFCA7) {
            nfc->device.data.uid_len = 7;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetSak);
            return true;
        } else if(event.event == SubmenuIndexNFCA4) {
            nfc->device.data.uid_len = 4;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetSak);
            return true;
        }
    }
    return false;
}

const void nfc_scene_set_type_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    submenu_clean(nfc->submenu);
}
