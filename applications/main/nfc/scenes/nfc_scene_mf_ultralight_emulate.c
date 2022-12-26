#include "../nfc_i.h"

#define NFC_MF_UL_DATA_NOT_CHANGED (0UL)
#define NFC_MF_UL_DATA_CHANGED (1UL)

bool nfc_mf_ultralight_emulate_worker_callback(NfcWorkerEvent event, void* context) {
    UNUSED(event);
    Nfc* nfc = context;

    scene_manager_set_scene_state(
        nfc->scene_manager, NfcSceneMfUltralightEmulate, NFC_MF_UL_DATA_CHANGED);
    return true;
}

void nfc_scene_mf_ultralight_emulate_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup view
    MfUltralightType type = nfc->dev->dev_data.mf_ul_data.type;
    bool is_ultralight = (type == MfUltralightTypeUL11) || (type == MfUltralightTypeUL21) ||
                         (type == MfUltralightTypeUnknown);
    Popup* popup = nfc->popup;
    popup_set_header(popup, "Emulating", 67, 13, AlignLeft, AlignTop);
    if(strcmp(nfc->dev->dev_name, "") != 0) {
        nfc_text_store_set(nfc, "%s", nfc->dev->dev_name);
    } else if(is_ultralight) {
        nfc_text_store_set(nfc, "MIFARE\nUltralight");
    } else {
        nfc_text_store_set(nfc, "MIFARE\nNTAG");
    }
    popup_set_icon(popup, 0, 3, &I_NFC_dolphin_emulation_47x61);
    popup_set_text(popup, nfc->text_store, 90, 28, AlignCenter, AlignTop);

    // Setup and start worker
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
    nfc_worker_start(
        nfc->worker,
        NfcWorkerStateMfUltralightEmulate,
        &nfc->dev->dev_data,
        nfc_mf_ultralight_emulate_worker_callback,
        nfc);
    nfc_blink_emulate_start(nfc);
}

bool nfc_scene_mf_ultralight_emulate_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        // Stop worker
        nfc_worker_stop(nfc->worker);
        // Check if data changed and save in shadow file
        if(scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfUltralightEmulate) ==
           NFC_MF_UL_DATA_CHANGED) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfUltralightEmulate, NFC_MF_UL_DATA_NOT_CHANGED);
            // Save shadow file
            if(furi_string_size(nfc->dev->load_path)) {
                nfc_device_save_shadow(nfc->dev, furi_string_get_cstr(nfc->dev->load_path));
            }
        }
        consumed = false;
    }
    return consumed;
}

void nfc_scene_mf_ultralight_emulate_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    popup_reset(nfc->popup);

    nfc_blink_stop(nfc);
}
