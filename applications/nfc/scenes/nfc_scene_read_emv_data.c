#include "../nfc_i.h"
#include <dolphin/dolphin.h>

void nfc_read_emv_data_worker_callback(NfcWorkerEvent event, void* context) {
    Nfc* nfc = (Nfc*)context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventWorkerExit);
}

void nfc_scene_read_emv_data_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    DOLPHIN_DEED(DolphinDeedNfcRead);

    // Setup view
    Popup* popup = nfc->popup;
    popup_set_header(popup, "Reading\nbank card", 70, 34, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinReceive_97x61);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
    // Clear emv data
    memset(&nfc->dev->dev_data.emv_data, 0, sizeof(nfc->dev->dev_data.emv_data));
    // Start worker
    nfc_worker_start(
        nfc->worker,
        NfcWorkerStateReadEMV,
        &nfc->dev->dev_data,
        nfc_read_emv_data_worker_callback,
        nfc);
}

bool nfc_scene_read_emv_data_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventWorkerExit) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneReadEmvDataSuccess, NFC_SEND_NOTIFICATION_TRUE);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneReadEmvDataSuccess);
            return true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        notification_message(nfc->notifications, &sequence_blink_blue_10);
        return true;
    }
    return false;
}

void nfc_scene_read_emv_data_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Stop worker
    nfc_worker_stop(nfc->worker);

    // Clear view
    Popup* popup = nfc->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
