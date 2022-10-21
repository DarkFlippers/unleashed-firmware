#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexEmulate,
    SubmenuIndexEditUid,
    SubmenuIndexRename,
    SubmenuIndexDelete,
    SubmenuIndexInfo,
    SubmenuIndexRestoreOriginal,
};

void nfc_scene_saved_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_saved_menu_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    if(nfc->dev->format == NfcDeviceSaveFormatUid ||
       nfc->dev->format == NfcDeviceSaveFormatMifareDesfire ||
       nfc->dev->format == NfcDeviceSaveFormatBankCard) {
        submenu_add_item(
            submenu,
            "Emulate UID",
            SubmenuIndexEmulate,
            nfc_scene_saved_menu_submenu_callback,
            nfc);
        if(nfc->dev->dev_data.protocol == NfcDeviceProtocolUnknown) {
            submenu_add_item(
                submenu,
                "Edit UID",
                SubmenuIndexEditUid,
                nfc_scene_saved_menu_submenu_callback,
                nfc);
        }
    } else if(
        nfc->dev->format == NfcDeviceSaveFormatMifareUl ||
        nfc->dev->format == NfcDeviceSaveFormatMifareClassic) {
        submenu_add_item(
            submenu, "Emulate", SubmenuIndexEmulate, nfc_scene_saved_menu_submenu_callback, nfc);
    }
    submenu_add_item(
        submenu, "Info", SubmenuIndexInfo, nfc_scene_saved_menu_submenu_callback, nfc);
    if(nfc->dev->shadow_file_exist) {
        submenu_add_item(
            submenu,
            "Restore to original",
            SubmenuIndexRestoreOriginal,
            nfc_scene_saved_menu_submenu_callback,
            nfc);
    }
    submenu_add_item(
        submenu, "Rename", SubmenuIndexRename, nfc_scene_saved_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Delete", SubmenuIndexDelete, nfc_scene_saved_menu_submenu_callback, nfc);
    submenu_set_selected_item(
        nfc->submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneSavedMenu));

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_saved_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    NfcDeviceData* dev_data = &nfc->dev->dev_data;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(nfc->scene_manager, NfcSceneSavedMenu, event.event);
        if(event.event == SubmenuIndexEmulate) {
            if(nfc->dev->format == NfcDeviceSaveFormatMifareUl) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightEmulate);
            } else if(nfc->dev->format == NfcDeviceSaveFormatMifareClassic) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicEmulate);
            } else {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneEmulateUid);
            }
            consumed = true;
        } else if(event.event == SubmenuIndexRename) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveName);
            consumed = true;
        } else if(event.event == SubmenuIndexEditUid) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetUid);
            consumed = true;
        } else if(event.event == SubmenuIndexDelete) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneDelete);
            consumed = true;
        } else if(event.event == SubmenuIndexInfo) {
            bool application_info_present = false;
            if(dev_data->protocol == NfcDeviceProtocolEMV) {
                application_info_present = true;
            } else if(
                dev_data->protocol == NfcDeviceProtocolMifareClassic ||
                dev_data->protocol == NfcDeviceProtocolMifareUl) {
                application_info_present = nfc_supported_card_verify_and_parse(dev_data);
            }

            if(application_info_present) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneDeviceInfo);
            } else {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneNfcDataInfo);
            }
            consumed = true;
        } else if(event.event == SubmenuIndexRestoreOriginal) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRestoreOriginalConfirm);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_saved_menu_on_exit(void* context) {
    Nfc* nfc = context;

    submenu_reset(nfc->submenu);
}
