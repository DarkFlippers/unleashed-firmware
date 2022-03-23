#include "../nfc_i.h"
#include <dolphin/dolphin.h>

void nfc_read_card_worker_callback(NfcWorkerEvent event, void* context) {
    Nfc* nfc = (Nfc*)context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventWorkerExit);
}

void nfc_scene_read_card_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;
    DOLPHIN_DEED(DolphinDeedNfcRead);

    // Setup view
    Popup* popup = nfc->popup;
    popup_set_header(popup, "Detecting\nNFC card", 70, 34, AlignLeft, AlignTop);
    popup_set_icon(popup, 0, 3, &I_RFIDDolphinReceive_97x61);

    // Start worker
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
    nfc_worker_start(
        nfc->worker, NfcWorkerStateDetect, &nfc->dev->dev_data, nfc_read_card_worker_callback, nfc);
}

bool nfc_scene_read_card_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventWorkerExit) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneReadCardSuccess);
            return true;
        }
    } else if(event.type == SceneManagerEventTypeTick) {
        notification_message(nfc->notifications, &sequence_blink_blue_10);
        return true;
    }
    return false;
}

void nfc_scene_read_card_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Stop worker
    nfc_worker_stop(nfc->worker);

    // Clear view
    Popup* popup = nfc->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
