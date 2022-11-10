#include "../nfc_i.h"
#include "nfc_worker_i.h"

enum SubmenuIndex {
    SubmenuIndexReadMifareClassic,
    SubmenuIndexReadMifareDesfire,
    SubmenuIndexReadMfUltralight,
    SubmenuIndexReadEMV,
    SubmenuIndexReadNFCA,
};

void nfc_scene_read_card_type_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_read_card_type_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(
        submenu,
        "Read Mifare Classic",
        SubmenuIndexReadMifareClassic,
        nfc_scene_read_card_type_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Read Mifare DESFire",
        SubmenuIndexReadMifareDesfire,
        nfc_scene_read_card_type_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Read NTAG/Ultralight",
        SubmenuIndexReadMfUltralight,
        nfc_scene_read_card_type_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Read EMV card",
        SubmenuIndexReadEMV,
        nfc_scene_read_card_type_submenu_callback,
        nfc);
    submenu_add_item(
        submenu,
        "Read NFC-A data",
        SubmenuIndexReadNFCA,
        nfc_scene_read_card_type_submenu_callback,
        nfc);
    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneReadCardType);
    submenu_set_selected_item(submenu, state);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_read_card_type_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexReadMifareClassic) {
            nfc->dev->dev_data.read_mode = NfcReadModeMfClassic;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRead);
            consumed = true;
        }
        if(event.event == SubmenuIndexReadMifareDesfire) {
            nfc->dev->dev_data.read_mode = NfcReadModeMfDesfire;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRead);
            consumed = true;
        }
        if(event.event == SubmenuIndexReadMfUltralight) {
            nfc->dev->dev_data.read_mode = NfcReadModeMfUltralight;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRead);
            consumed = true;
        }
        if(event.event == SubmenuIndexReadEMV) {
            nfc->dev->dev_data.read_mode = NfcReadModeEMV;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRead);
            consumed = true;
        }
        if(event.event == SubmenuIndexReadNFCA) {
            nfc->dev->dev_data.read_mode = NfcReadModeNFCA;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRead);
            consumed = true;
        }
        scene_manager_set_scene_state(nfc->scene_manager, NfcSceneReadCardType, event.event);
    }
    return consumed;
}

void nfc_scene_read_card_type_on_exit(void* context) {
    Nfc* nfc = context;

    submenu_reset(nfc->submenu);
}
