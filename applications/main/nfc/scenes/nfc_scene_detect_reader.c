#include "../nfc_i.h"
#include <dolphin/dolphin.h>

bool nfc_detect_reader_worker_callback(NfcWorkerEvent event, void* context) {
    UNUSED(event);
    furi_assert(context);
    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, event);
    return true;
}

void nfc_scene_detect_reader_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
}

void nfc_scene_detect_reader_on_enter(void* context) {
    Nfc* nfc = context;
    DOLPHIN_DEED(DolphinDeedNfcEmulate);

    detect_reader_set_callback(nfc->detect_reader, nfc_scene_detect_reader_callback, nfc);
    nfc_worker_start(
        nfc->worker,
        NfcWorkerStateAnalyzeReader,
        &nfc->dev->dev_data,
        nfc_detect_reader_worker_callback,
        nfc);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDetectReader);

    nfc_blink_read_start(nfc);
}

bool nfc_scene_detect_reader_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            nfc_worker_stop(nfc->worker);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfkeyNoncesInfo);
            consumed = true;
        } else if(event.event == NfcWorkerEventDetectReaderMfkeyCollected) {
            detect_reader_inc_nonce_cnt(nfc->detect_reader);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_detect_reader_on_exit(void* context) {
    Nfc* nfc = context;

    // Stop worker
    nfc_worker_stop(nfc->worker);

    // Clear view
    detect_reader_reset(nfc->detect_reader);

    nfc_blink_stop(nfc);
}
