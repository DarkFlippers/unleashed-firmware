#include "../nfc_i.h"

#define NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX (10U)

static const NotificationSequence sequence_detect_reader = {
    &message_green_255,
    &message_blue_255,
    &message_do_not_reset,
    NULL,
};

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

    detect_reader_set_callback(nfc->detect_reader, nfc_scene_detect_reader_callback, nfc);
    detect_reader_set_nonces_max(nfc->detect_reader, NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX);
    NfcDeviceData* dev_data = &nfc->dev->dev_data;
    if(dev_data->nfc_data.uid_len) {
        detect_reader_set_uid(
            nfc->detect_reader, dev_data->nfc_data.uid, dev_data->nfc_data.uid_len);
    }

    // Store number of collected nonces in scene state
    scene_manager_set_scene_state(nfc->scene_manager, NfcSceneDetectReader, 0);
    notification_message(nfc->notifications, &sequence_detect_reader);

    nfc_worker_start(
        nfc->worker,
        NfcWorkerStateAnalyzeReader,
        &nfc->dev->dev_data,
        nfc_detect_reader_worker_callback,
        nfc);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDetectReader);
}

bool nfc_scene_detect_reader_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;
    uint32_t nonces_collected =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneDetectReader);

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            nfc_worker_stop(nfc->worker);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfkeyNoncesInfo);
            consumed = true;
        } else if(event.event == NfcWorkerEventDetectReaderMfkeyCollected) {
            nonces_collected += 2;
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneDetectReader, nonces_collected);
            detect_reader_set_nonces_collected(nfc->detect_reader, nonces_collected);
            if(nonces_collected >= NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX) {
                detect_reader_set_state(nfc->detect_reader, DetectReaderStateDone);
                nfc_blink_stop(nfc);
                notification_message(nfc->notifications, &sequence_single_vibro);
                notification_message(nfc->notifications, &sequence_set_green_255);
                nfc_worker_stop(nfc->worker);
            }
            consumed = true;
        } else if(event.event == NfcWorkerEventDetectReaderDetected) {
            if(nonces_collected < NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX) {
                notification_message(nfc->notifications, &sequence_blink_start_cyan);
                detect_reader_set_state(nfc->detect_reader, DetectReaderStateReaderDetected);
            }
        } else if(event.event == NfcWorkerEventDetectReaderLost) {
            if(nonces_collected < NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX) {
                nfc_blink_stop(nfc);
                notification_message(nfc->notifications, &sequence_detect_reader);
                detect_reader_set_state(nfc->detect_reader, DetectReaderStateReaderLost);
            }
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

    // Stop notifications
    nfc_blink_stop(nfc);
    notification_message(nfc->notifications, &sequence_reset_green);
}
