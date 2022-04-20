#include "../nfc_i.h"

enum SubmenuDebugIndex {
    SubmenuDebugIndexField,
    SubmenuDebugIndexApdu,
};

void nfc_scene_debug_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_debug_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu, "Field", SubmenuDebugIndexField, nfc_scene_debug_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Apdu", SubmenuDebugIndexApdu, nfc_scene_debug_submenu_callback, nfc);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneDebug));

    nfc_device_clear(nfc->dev);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_debug_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuDebugIndexField) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneDebug, SubmenuDebugIndexField);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneField);
            consumed = true;
        } else if(event.event == SubmenuDebugIndexApdu) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneDebug, SubmenuDebugIndexApdu);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneEmulateApduSequence);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_debug_on_exit(void* context) {
    Nfc* nfc = context;

    submenu_reset(nfc->submenu);
}
