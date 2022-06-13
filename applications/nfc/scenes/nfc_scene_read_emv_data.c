#include "../nfc_i.h"
#include <dolphin/dolphin.h>

void nfc_read_emv_data_worker_callback(NfcWorkerEvent event, void* context) {
    UNUSED(event);
    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventWorkerExit);
}

void nfc_scene_read_emv_data_on_enter(void* context) {
    Nfc* nfc = context;
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
        NfcWorkerStateReadEMVData,
        &nfc->dev->dev_data,
        nfc_read_emv_data_worker_callback,
        nfc);

    nfc_blink_start(nfc);
}

bool nfc_scene_read_emv_data_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventWorkerExit) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneReadEmvDataSuccess, NFC_SEND_NOTIFICATION_TRUE);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneReadEmvDataSuccess);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        consumed = true;
    }
    return consumed;
}

void nfc_scene_read_emv_data_on_exit(void* context) {
    Nfc* nfc = context;

    // Stop worker
    nfc_worker_stop(nfc->worker);
    // Clear view
    popup_reset(nfc->popup);

    nfc_blink_stop(nfc);
}
