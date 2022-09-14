#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexUnlock,
    SubmenuIndexSave,
    SubmenuIndexEmulate,
    SubmenuIndexInfo,
};

void nfc_scene_mf_ultralight_menu_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_mf_ultralight_menu_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;
    MfUltralightData* data = &nfc->dev->dev_data.mf_ul_data;

    if(data->data_read != data->data_size) {
        submenu_add_item(
            submenu,
            "Unlock With Password",
            SubmenuIndexUnlock,
            nfc_scene_mf_ultralight_menu_submenu_callback,
            nfc);
    }
    submenu_add_item(
        submenu, "Save", SubmenuIndexSave, nfc_scene_mf_ultralight_menu_submenu_callback, nfc);
    submenu_add_item(
        submenu,
        "Emulate",
        SubmenuIndexEmulate,
        nfc_scene_mf_ultralight_menu_submenu_callback,
        nfc);
    submenu_add_item(
        submenu, "Info", SubmenuIndexInfo, nfc_scene_mf_ultralight_menu_submenu_callback, nfc);

    submenu_set_selected_item(
        nfc->submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfUltralightMenu));

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_mf_ultralight_menu_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexSave) {
            nfc->dev->format = NfcDeviceSaveFormatMifareUl;
            // Clear device name
            nfc_device_set_name(nfc->dev, "");
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveName);
            consumed = true;
        } else if(event.event == SubmenuIndexEmulate) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightEmulate);
            consumed = true;
        } else if(event.event == SubmenuIndexUnlock) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockMenu);
            consumed = true;
        } else if(event.event == SubmenuIndexInfo) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneNfcDataInfo);
            consumed = true;
        }
        scene_manager_set_scene_state(nfc->scene_manager, NfcSceneMfUltralightMenu, event.event);

    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_previous_scene(nfc->scene_manager);
    }

    return consumed;
}

void nfc_scene_mf_ultralight_menu_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    submenu_reset(nfc->submenu);
}
