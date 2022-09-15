#include "../nfc_i.h"
#include <dolphin/dolphin.h>

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
    DOLPHIN_DEED(DolphinDeedNfcEmulate);

    // Setup view
    Popup* popup = nfc->popup;
    if(strcmp(nfc->dev->dev_name, "")) {
        nfc_text_store_set(nfc, "Emulating\n%s", nfc->dev->dev_name);
    } else {
        nfc_text_store_set(nfc, "Emulating\nMf Ultralight", nfc->dev->dev_name);
    }
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinSend_97x61);
    popup_set_header(popup, nfc->text_store, 56, 31, AlignLeft, AlignTop);

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
            nfc_device_save_shadow(nfc->dev, nfc->dev->dev_name);
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
