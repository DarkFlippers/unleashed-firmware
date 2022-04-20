#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexSave,
};

void nfc_scene_mifare_desfire_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_mifare_desfire_menu_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu, "Save", SubmenuIndexSave, nfc_scene_mifare_desfire_menu_submenu_callback, nfc);
    submenu_set_selected_item(
        nfc->submenu,
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMifareDesfireMenu));

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mifare_desfire_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexSave) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMifareDesfireMenu, SubmenuIndexSave);
            nfc->dev->format = NfcDeviceSaveFormatMifareDesfire;
            // Clear device name
            nfc_device_set_name(nfc->dev, "");
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveName);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_mifare_desfire_menu_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    submenu_reset(nfc->submenu);
}
