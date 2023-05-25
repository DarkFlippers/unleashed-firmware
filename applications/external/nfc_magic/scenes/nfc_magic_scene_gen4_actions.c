#include "../nfc_magic_i.h"
enum SubmenuIndex {
    SubmenuIndexWrite,
    SubmenuIndexChangePassword,
    SubmenuIndexWipe,
};

void nfc_magic_scene_gen4_actions_submenu_callback(void* context, uint32_t index) {
    NfcMagic* nfc_magic = context;
    view_dispatcher_send_custom_event(nfc_magic->view_dispatcher, index);
}

void nfc_magic_scene_gen4_actions_on_enter(void* context) {
    NfcMagic* nfc_magic = context;

    Submenu* submenu = nfc_magic->submenu;
    submenu_add_item(
        submenu,
        "Write",
        SubmenuIndexWrite,
        nfc_magic_scene_gen4_actions_submenu_callback,
        nfc_magic);
    submenu_add_item(
        submenu,
        "Change password",
        SubmenuIndexChangePassword,
        nfc_magic_scene_gen4_actions_submenu_callback,
        nfc_magic);
    submenu_add_item(
        submenu,
        "Wipe",
        SubmenuIndexWipe,
        nfc_magic_scene_gen4_actions_submenu_callback,
        nfc_magic);

    submenu_set_selected_item(
        submenu,
        scene_manager_get_scene_state(nfc_magic->scene_manager, NfcMagicSceneGen4Actions));
    view_dispatcher_switch_to_view(nfc_magic->view_dispatcher, NfcMagicViewMenu);
}

bool nfc_magic_scene_gen4_actions_on_event(void* context, SceneManagerEvent event) {
    NfcMagic* nfc_magic = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexWrite) {
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneFileSelect);
            consumed = true;
        } else if(event.event == SubmenuIndexChangePassword) {
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneNewKeyInput);
            consumed = true;
        } else if(event.event == SubmenuIndexWipe) {
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneWipe);
            consumed = true;
        }
        scene_manager_set_scene_state(
            nfc_magic->scene_manager, NfcMagicSceneGen4Actions, event.event);
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            nfc_magic->scene_manager, NfcMagicSceneStart);
    }

    return consumed;
}

void nfc_magic_scene_gen4_actions_on_exit(void* context) {
    NfcMagic* nfc_magic = context;
    submenu_reset(nfc_magic->submenu);
}
