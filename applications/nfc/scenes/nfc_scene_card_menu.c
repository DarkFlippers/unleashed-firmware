#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexRunApp,
    SubmenuIndexChooseScript,
    SubmenuIndexEmulate,
    SubmenuIndexSave,
};

void nfc_scene_card_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_card_menu_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    if(nfc->dev->dev_data.protocol > NfcDeviceProtocolUnknown) {
        submenu_add_item(
            submenu,
            "Run Compatible App",
            SubmenuIndexRunApp,
            nfc_scene_card_menu_submenu_callback,
            nfc);
    }
    submenu_add_item(
        submenu,
        "Additional reading scripts",
        SubmenuIndexChooseScript,
        nfc_scene_card_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu, "Emulate UID", SubmenuIndexEmulate, nfc_scene_card_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Save UID", SubmenuIndexSave, nfc_scene_card_menu_submenu_callback, nfc);
    submenu_set_selected_item(
        nfc->submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneCardMenu));

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_card_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexRunApp) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneCardMenu, SubmenuIndexRunApp);
            if(nfc->dev->dev_data.protocol == NfcDeviceProtocolMifareUl) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneReadMifareUl);
            } else if(nfc->dev->dev_data.protocol == NfcDeviceProtocolMifareDesfire) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneReadMifareDesfire);
            } else if(nfc->dev->dev_data.protocol == NfcDeviceProtocolEMV) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneReadEmvApp);
            } else if(nfc->dev->dev_data.protocol == NfcDeviceProtocolMifareClassic) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneReadMifareClassic);
            }
            consumed = true;
        } else if(event.event == SubmenuIndexChooseScript) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneCardMenu, SubmenuIndexChooseScript);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneScriptsMenu);
            consumed = true;
        } else if(event.event == SubmenuIndexEmulate) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneCardMenu, SubmenuIndexEmulate);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneEmulateUid);
            consumed = true;
        } else if(event.event == SubmenuIndexSave) {
            scene_manager_set_scene_state(nfc->scene_manager, NfcSceneCardMenu, SubmenuIndexSave);
            nfc->dev->format = NfcDeviceSaveFormatUid;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveName);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed =
            scene_manager_search_and_switch_to_previous_scene(nfc->scene_manager, NfcSceneStart);
    }

    return consumed;
}

void nfc_scene_card_menu_on_exit(void* context) {
    Nfc* nfc = context;

    submenu_reset(nfc->submenu);
}
