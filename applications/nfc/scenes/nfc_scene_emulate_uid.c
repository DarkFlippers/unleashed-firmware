#include "../nfc_i.h"

const void nfc_scene_emulate_uid_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    Popup* popup = nfc->popup;
    NfcDeviceCommonData* data = &nfc->dev.dev_data.nfc_data;

    if(strcmp(nfc->dev.dev_name, "")) {
        nfc_text_store_set(nfc, "%s", nfc->dev.dev_name);
    } else if(data->uid_len == 4) {
        nfc_text_store_set(
            nfc, "%02X %02X %02X %02X", data->uid[0], data->uid[1], data->uid[2], data->uid[3]);
    } else if(data->uid_len == 7) {
        nfc_text_store_set(
            nfc,
            "%02X %02X %02X %02X\n%02X %02X %02X",
            data->uid[0],
            data->uid[1],
            data->uid[2],
            data->uid[3],
            data->uid[4],
            data->uid[5],
            data->uid[6]);
    }

    popup_set_icon(popup, 0, 3, &I_RFIDDolphinSend_97x61);
    popup_set_header(popup, "Emulating UID", 56, 31, AlignLeft, AlignTop);
    popup_set_text(popup, nfc->text_store, 56, 43, AlignLeft, AlignTop);

    // Setup and start worker

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
    nfc_worker_start(nfc->worker, NfcWorkerStateEmulate, &nfc->dev.dev_data, NULL, nfc);
}

const bool nfc_scene_emulate_uid_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeTick) {
        notification_message(nfc->notifications, &sequence_blink_blue_10);
        return true;
    }
    return false;
}

const void nfc_scene_emulate_uid_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Stop worker
    nfc_worker_stop(nfc->worker);

    // Clear view
    Popup* popup = nfc->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
