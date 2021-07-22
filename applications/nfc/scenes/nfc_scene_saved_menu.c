#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexEmulate,
    SubmenuIndexEdit,
    SubmenuIndexDelete,
    SubmenuIndexInfo,
};

void nfc_scene_saved_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

const void nfc_scene_saved_menu_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    Submenu* submenu = nfc->submenu;

    if(nfc->dev.format != NfcDeviceSaveFormatBankCard) {
        submenu_add_item(
            submenu, "Emulate", SubmenuIndexEmulate, nfc_scene_saved_menu_submenu_callback, nfc);
    }
    submenu_add_item(
        submenu, "Edit UID and name", SubmenuIndexEdit, nfc_scene_saved_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Delete", SubmenuIndexDelete, nfc_scene_saved_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Info", SubmenuIndexInfo, nfc_scene_saved_menu_submenu_callback, nfc);
    submenu_set_selected_item(
        nfc->submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneSavedMenu));

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

const bool nfc_scene_saved_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexEmulate) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneSavedMenu, SubmenuIndexEmulate);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneEmulateUid);
            return true;
        } else if(event.event == SubmenuIndexEdit) {
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneSavedMenu, SubmenuIndexEdit);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetUid);
            return true;
        } else if(event.event == SubmenuIndexDelete) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneSavedMenu, SubmenuIndexDelete);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneDelete);
            return true;
        } else if(event.event == SubmenuIndexInfo) {
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneSavedMenu, SubmenuIndexInfo);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneDeviceInfo);
            return true;
        }
    }

    return false;
}

const void nfc_scene_saved_menu_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    submenu_clean(nfc->submenu);
}
