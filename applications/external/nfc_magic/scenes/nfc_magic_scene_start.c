#include "../nfc_magic_i.h"
enum SubmenuIndex {
    SubmenuIndexCheck,
    SubmenuIndexAuthenticateGen4,
};

void nfc_magic_scene_start_submenu_callback(void* context, uint32_t index) {
    NfcMagic* nfc_magic = context;
    view_dispatcher_send_custom_event(nfc_magic->view_dispatcher, index);
}

void nfc_magic_scene_start_on_enter(void* context) {
    NfcMagic* nfc_magic = context;

    Submenu* submenu = nfc_magic->submenu;
    submenu_add_item(
        submenu,
        "Check Magic Tag",
        SubmenuIndexCheck,
        nfc_magic_scene_start_submenu_callback,
        nfc_magic);
    submenu_add_item(
        submenu,
        "Authenticate Gen4",
        SubmenuIndexAuthenticateGen4,
        nfc_magic_scene_start_submenu_callback,
        nfc_magic);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(nfc_magic->scene_manager, NfcMagicSceneStart));
    view_dispatcher_switch_to_view(nfc_magic->view_dispatcher, NfcMagicViewMenu);
}

bool nfc_magic_scene_start_on_event(void* context, SceneManagerEvent event) {
    NfcMagic* nfc_magic = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexCheck) {
            nfc_magic->dev->password = MAGIC_GEN4_DEFAULT_PWD;
            scene_manager_set_scene_state(
                nfc_magic->scene_manager, NfcMagicSceneStart, SubmenuIndexCheck);
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneCheck);
            consumed = true;
        } else if(event.event == SubmenuIndexAuthenticateGen4) {
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneKeyInput);
        }
    }

    return consumed;
}

void nfc_magic_scene_start_on_exit(void* context) {
    NfcMagic* nfc_magic = context;
    submenu_reset(nfc_magic->submenu);
}
