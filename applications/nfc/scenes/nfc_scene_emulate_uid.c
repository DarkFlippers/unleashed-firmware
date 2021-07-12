#include "../nfc_i.h"

const void nfc_scene_emulate_uid_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    Popup* popup = nfc->popup;
    NfcDeviceData* data = &nfc->device.data;

    if(strcmp(nfc->device.dev_name, "")) {
        nfc_text_store_set(nfc, "%s", nfc->device.dev_name);
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

    nfc_worker_set_emulation_params(nfc->nfc_common.worker, data);
    nfc_worker_start(
        nfc->nfc_common.worker, NfcWorkerStateEmulate, &nfc->nfc_common.worker_result, NULL, nfc);
    view_dispatcher_switch_to_view(nfc->nfc_common.view_dispatcher, NfcViewPopup);
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
    nfc_worker_stop(nfc->nfc_common.worker);

    // Clear view
    Popup* popup = nfc->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
