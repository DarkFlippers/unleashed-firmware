#include "../nfc_i.h"
#include <dolphin/dolphin.h>

#define NFC_MF_UL_DATA_NOT_CHANGED (0UL)
#define NFC_MF_UL_DATA_CHANGED (1UL)

void nfc_emulate_mifare_ul_worker_callback(NfcWorkerEvent event, void* context) {
    Nfc* nfc = (Nfc*)context;
    scene_manager_set_scene_state(
        nfc->scene_manager, NfcSceneEmulateMifareUl, NFC_MF_UL_DATA_CHANGED);
}

void nfc_scene_emulate_mifare_ul_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    DOLPHIN_DEED(DolphinDeedNfcEmulate);

    // Setup view
    Popup* popup = nfc->popup;
    if(strcmp(nfc->dev->dev_name, "")) {
        nfc_text_store_set(nfc, "%s", nfc->dev->dev_name);
    }
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinSend_97x61);
    popup_set_header(popup, "Emulating\nMf Ultralight", 56, 31, AlignLeft, AlignTop);

    // Setup and start worker
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
    nfc_worker_start(
        nfc->worker,
        NfcWorkerStateEmulateMifareUl,
        &nfc->dev->dev_data,
        nfc_emulate_mifare_ul_worker_callback,
        nfc);
}

bool nfc_scene_emulate_mifare_ul_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        notification_message(nfc->notifications, &sequence_blink_blue_10);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        // Stop worker
        nfc_worker_stop(nfc->worker);
        // Check if data changed and save in shadow file
        if(scene_manager_get_scene_state(nfc->scene_manager, NfcSceneEmulateMifareUl) ==
           NFC_MF_UL_DATA_CHANGED) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneEmulateMifareUl, NFC_MF_UL_DATA_NOT_CHANGED);
            nfc_device_save_shadow(nfc->dev, nfc->dev->dev_name);
        }
        consumed = false;
    }
    return consumed;
}

void nfc_scene_emulate_mifare_ul_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear view
    Popup* popup = nfc->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
