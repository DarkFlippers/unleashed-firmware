#include "../nfc_i.h"

enum SubmenuIndex {
    SubmenuIndexRead,
    SubmenuIndexDetectReader,
    SubmenuIndexSaved,
    SubmenuIndexExtraAction,
    SubmenuIndexAddManualy,
    SubmenuIndexDebug,
};

void nfc_scene_start_submenu_callback(void* context, uint32_t index) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, index);
}

void nfc_scene_start_on_enter(void* context) {
    Nfc* nfc = context;
    Submenu* submenu = nfc->submenu;

    submenu_add_item(submenu, "Read", SubmenuIndexRead, nfc_scene_start_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Detect Reader", SubmenuIndexDetectReader, nfc_scene_start_submenu_callback, nfc);
    submenu_add_item(submenu, "Saved", SubmenuIndexSaved, nfc_scene_start_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Extra Actions", SubmenuIndexExtraAction, nfc_scene_start_submenu_callback, nfc);
    submenu_add_item(
        submenu, "Add Manually", SubmenuIndexAddManualy, nfc_scene_start_submenu_callback, nfc);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        submenu_add_item(
            submenu, "Debug", SubmenuIndexDebug, nfc_scene_start_submenu_callback, nfc);
    }

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(nfc->scene_manager, NfcSceneStart));

    nfc_device_clear(nfc->dev);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewMenu);
}

bool nfc_scene_start_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubmenuIndexRead) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRead);
            consumed = true;
        } else if(event.event == SubmenuIndexDetectReader) {
            bool sd_exist = storage_sd_status(nfc->dev->storage) == FSE_OK;
            if(sd_exist) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneDetectReader);
            } else {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneDictNotFound);
            }
            consumed = true;
        } else if(event.event == SubmenuIndexSaved) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneFileSelect);
            consumed = true;
        } else if(event.event == SubmenuIndexExtraAction) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneExtraActions);
            consumed = true;
        } else if(event.event == SubmenuIndexAddManualy) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSetType);
            consumed = true;
        } else if(event.event == SubmenuIndexDebug) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneDebug);
            consumed = true;
        }
        scene_manager_set_scene_state(nfc->scene_manager, NfcSceneStart, event.event);
    }
    return consumed;
}

void nfc_scene_start_on_exit(void* context) {
    Nfc* nfc = context;

    submenu_reset(nfc->submenu);
}
