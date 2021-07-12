#include "../nfc_i.h"

void nfc_read_mifare_ul_worker_callback(void* context) {
    Nfc* nfc = (Nfc*)context;
    view_dispatcher_send_custom_event(nfc->nfc_common.view_dispatcher, NfcEventMifareUl);
}

const void nfc_scene_read_mifare_ul_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    Popup* popup = nfc->popup;
    popup_set_header(popup, "Detecting\nultralight", 70, 34, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinReceive_97x61);

    // Start worker
    nfc_worker_start(
        nfc->nfc_common.worker,
        NfcWorkerStateReadMfUltralight,
        &nfc->nfc_common.worker_result,
        nfc_read_mifare_ul_worker_callback,
        nfc);
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewPopup);
}

const bool nfc_scene_read_mifare_ul_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcEventMifareUl) {
            nfc->device.data = nfc->nfc_common.worker_result.nfc_detect_data;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneReadMifareUlSuccess);
            return true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        notification_message(nfc->notifications, &sequence_blink_blue_10);
        return true;
    }
    return false;
}

const void nfc_scene_read_mifare_ul_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Stop worker
    nfc_worker_stop(nfc->nfc_common.worker);

    // Clear view
    Popup* popup = nfc->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
