#include "../nfc_app_i.h"

#include <nfc/protocols/mf_classic/mf_classic_listener.h>

#define NXP_MANUFACTURER_ID (0x04)

#define NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX        (10U)
#define NFC_SCENE_DETECT_READER_WAIT_NONCES_TIMEOUT_MS (1000)

static const NotificationSequence sequence_detect_reader = {
    &message_green_255,
    &message_blue_255,
    &message_do_not_reset,
    NULL,
};

void nfc_scene_mf_classic_detect_reader_view_callback(void* context) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventViewExit);
}

NfcCommand nfc_scene_mf_classic_detect_listener_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfClassic);

    NfcApp* instance = context;
    MfClassicListenerEvent* mfc_event = event.event_data;

    if(mfc_event->type == MfClassicListenerEventTypeAuthContextPartCollected) {
        MfClassicAuthContext* auth_ctx = &mfc_event->data->auth_context;
        mfkey32_logger_add_nonce(instance->mfkey32_logger, auth_ctx);
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWorkerUpdate);
    }

    return NfcCommandContinue;
}

void nfc_scene_mf_classic_timer_callback(void* context) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventTimerExpired);
}

void nfc_scene_mf_classic_detect_reader_on_enter(void* context) {
    NfcApp* instance = context;

    if(nfc_device_get_protocol(instance->nfc_device) == NfcProtocolInvalid) {
        Iso14443_3aData iso3_data = {
            .uid_len = 7,
            .uid = {0},
            .atqa = {0x44, 0x00},
            .sak = 0x08,
        };
        iso3_data.uid[0] = NXP_MANUFACTURER_ID;
        furi_hal_random_fill_buf(&iso3_data.uid[1], iso3_data.uid_len - 1);
        MfClassicData* mfc_data = mf_classic_alloc();

        mfc_data->type = MfClassicType4k;
        iso14443_3a_copy(mfc_data->iso14443_3a_data, &iso3_data);
        nfc_device_set_data(instance->nfc_device, NfcProtocolMfClassic, mfc_data);

        mf_classic_free(mfc_data);
    }

    const Iso14443_3aData* iso3_data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolIso14443_3a);
    uint32_t cuid = iso14443_3a_get_cuid(iso3_data);

    instance->mfkey32_logger = mfkey32_logger_alloc(cuid);
    instance->timer =
        furi_timer_alloc(nfc_scene_mf_classic_timer_callback, FuriTimerTypeOnce, instance);

    detect_reader_set_nonces_max(instance->detect_reader, NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX);
    detect_reader_set_callback(
        instance->detect_reader, nfc_scene_mf_classic_detect_reader_view_callback, instance);

    notification_message(instance->notifications, &sequence_detect_reader);

    instance->listener = nfc_listener_alloc(
        instance->nfc,
        NfcProtocolMfClassic,
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfClassic));
    nfc_listener_start(
        instance->listener, nfc_scene_mf_classic_detect_listener_callback, instance);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewDetectReader);
}

bool nfc_scene_mf_classic_detect_reader_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventWorkerUpdate) {
            furi_timer_stop(instance->timer);
            notification_message(instance->notifications, &sequence_blink_start_cyan);

            size_t nonces_pairs = 2 * mfkey32_logger_get_params_num(instance->mfkey32_logger);
            detect_reader_set_state(instance->detect_reader, DetectReaderStateReaderDetected);
            detect_reader_set_nonces_collected(instance->detect_reader, nonces_pairs);
            if(nonces_pairs >= NFC_SCENE_DETECT_READER_PAIR_NONCES_MAX) {
                if(instance->listener) {
                    nfc_listener_stop(instance->listener);
                    nfc_listener_free(instance->listener);
                    instance->listener = NULL;
                }
                detect_reader_set_state(instance->detect_reader, DetectReaderStateDone);
                nfc_blink_stop(instance);
                notification_message(instance->notifications, &sequence_single_vibro);
                notification_message(instance->notifications, &sequence_set_green_255);
            } else {
                furi_timer_start(instance->timer, NFC_SCENE_DETECT_READER_WAIT_NONCES_TIMEOUT_MS);
            }
            consumed = true;
        } else if(event.event == NfcCustomEventTimerExpired) {
            detect_reader_set_state(instance->detect_reader, DetectReaderStateReaderLost);
            nfc_blink_stop(instance);
            notification_message(instance->notifications, &sequence_detect_reader);
        } else if(event.event == NfcCustomEventViewExit) {
            if(instance->listener) {
                nfc_listener_stop(instance->listener);
                nfc_listener_free(instance->listener);
                instance->listener = NULL;
            }
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfClassicMfkeyNoncesInfo);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(instance->listener) {
            nfc_listener_stop(instance->listener);
            nfc_listener_free(instance->listener);
            instance->listener = NULL;
        }
        mfkey32_logger_free(instance->mfkey32_logger);
        if(scene_manager_has_previous_scene(instance->scene_manager, NfcSceneSaveSuccess)) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcSceneStart);
        } else if(scene_manager_has_previous_scene(instance->scene_manager, NfcSceneReadSuccess)) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcSceneReadSuccess);
        }
    }

    return consumed;
}

void nfc_scene_mf_classic_detect_reader_on_exit(void* context) {
    NfcApp* instance = context;

    // Clear view
    detect_reader_reset(instance->detect_reader);

    furi_timer_stop(instance->timer);
    furi_timer_free(instance->timer);

    // Stop notifications
    nfc_blink_stop(instance);
    notification_message(instance->notifications, &sequence_reset_green);
}
