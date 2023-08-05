#include "../nfc_i.h"
#include <core/common_defines.h>

void nfc_scene_emulate_apdu_sequence_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup view
    Popup* popup = nfc->popup;
    popup_set_header(popup, "Run APDU reader", 64, 31, AlignCenter, AlignTop);

    // Setup and start worker
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
    nfc_worker_start(nfc->worker, NfcWorkerStateEmulateApdu, &nfc->dev->dev_data, NULL, nfc);

    nfc_blink_emulate_start(nfc);
}

bool nfc_scene_emulate_apdu_sequence_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    bool consumed = false;
    return consumed;
}

void nfc_scene_emulate_apdu_sequence_on_exit(void* context) {
    Nfc* nfc = context;

    // Stop worker
    nfc_worker_stop(nfc->worker);
    // Clear view
    popup_reset(nfc->popup);

    nfc_blink_stop(nfc);
}
