#include "../nfc_app_i.h"

enum SubmenuIndex {
    SubmenuIndexSlixUnlockMenuManual,
    SubmenuIndexSlixUnlockMenuTonieBox,
};

void nfc_scene_slix_unlock_menu_submenu_callback(void* context, uint32_t index) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, index);
}

void nfc_scene_slix_unlock_menu_on_enter(void* context) {
    NfcApp* instance = context;
    Submenu* submenu = instance->submenu;

    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneSlixUnlockMenu);
    submenu_add_item(
        submenu,
        "Enter Password Manually",
        SubmenuIndexSlixUnlockMenuManual,
        nfc_scene_slix_unlock_menu_submenu_callback,
        instance);
    submenu_add_item(
        submenu,
        "Auth As TommyBox",
        SubmenuIndexSlixUnlockMenuTonieBox,
        nfc_scene_slix_unlock_menu_submenu_callback,
        instance);
    submenu_set_selected_item(submenu, state);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_slix_unlock_menu_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexSlixUnlockMenuManual) {
            slix_unlock_set_method(instance->slix_unlock, SlixUnlockMethodManual);
            scene_manager_next_scene(instance->scene_manager, NfcSceneSlixKeyInput);
            consumed = true;
        } else if(event.event == SubmenuIndexSlixUnlockMenuTonieBox) {
            slix_unlock_set_method(instance->slix_unlock, SlixUnlockMethodTonieBox);
            scene_manager_next_scene(instance->scene_manager, NfcSceneSlixUnlock);
            consumed = true;
        }
        scene_manager_set_scene_state(
            instance->scene_manager, NfcSceneSlixUnlockMenu, event.event);
    }
    return consumed;
}

void nfc_scene_slix_unlock_menu_on_exit(void* context) {
    NfcApp* instance = context;

    submenu_reset(instance->submenu);
}
