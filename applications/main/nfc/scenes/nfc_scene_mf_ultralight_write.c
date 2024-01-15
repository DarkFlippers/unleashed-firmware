#include "../nfc_app_i.h"

#include <nfc/protocols/mf_ultralight/mf_ultralight_poller.h>

enum {
    NfcSceneMfUltralightWriteStateCardSearch,
    NfcSceneMfUltralightWriteStateCardFound,
};

NfcCommand nfc_scene_mf_ultralight_write_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.protocol == NfcProtocolMfUltralight);

    NfcCommand command = NfcCommandContinue;
    NfcApp* instance = context;
    MfUltralightPollerEvent* mfu_event = event.event_data;

    if(mfu_event->type == MfUltralightPollerEventTypeRequestMode) {
        mfu_event->data->poller_mode = MfUltralightPollerModeWrite;
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventCardDetected);
    } else if(mfu_event->type == MfUltralightPollerEventTypeAuthRequest) {
        mfu_event->data->auth_context.skip_auth = true;
    } else if(mfu_event->type == MfUltralightPollerEventTypeRequestWriteData) {
        mfu_event->data->write_data =
            nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
    } else if(mfu_event->type == MfUltralightPollerEventTypeCardMismatch) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventWrongCard);
        command = NfcCommandStop;
    } else if(mfu_event->type == MfUltralightPollerEventTypeCardLocked) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
        command = NfcCommandStop;
    } else if(mfu_event->type == MfUltralightPollerEventTypeWriteFail) {
        command = NfcCommandStop;
    } else if(mfu_event->type == MfUltralightPollerEventTypeWriteSuccess) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
    }
    return command;
}

static void nfc_scene_mf_ultralight_write_setup_view(NfcApp* instance) {
    Popup* popup = instance->popup;
    popup_reset(popup);
    uint32_t state =
        scene_manager_get_scene_state(instance->scene_manager, NfcSceneMfUltralightWrite);

    if(state == NfcSceneMfUltralightWriteStateCardSearch) {
        popup_set_header(instance->popup, "Writing", 95, 20, AlignCenter, AlignCenter);
        popup_set_text(
            instance->popup, "Apply the initial\ncard only", 95, 38, AlignCenter, AlignCenter);
        popup_set_icon(instance->popup, 0, 8, &I_NFC_manual_60x50);
    } else {
        popup_set_header(popup, "Writing\nDon't move...", 52, 32, AlignLeft, AlignCenter);
        popup_set_icon(popup, 12, 23, &A_Loading_24);
    }

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);
}

void nfc_scene_mf_ultralight_write_on_enter(void* context) {
    NfcApp* instance = context;
    dolphin_deed(DolphinDeedNfcEmulate);

    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightWrite,
        NfcSceneMfUltralightWriteStateCardSearch);
    nfc_scene_mf_ultralight_write_setup_view(instance);

    // Setup and start worker
    FURI_LOG_D("WMFU", "Card searching...");
    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolMfUltralight);
    nfc_poller_start(instance->poller, nfc_scene_mf_ultralight_write_worker_callback, instance);

    nfc_blink_emulate_start(instance);
}

bool nfc_scene_mf_ultralight_write_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventCardDetected) {
            scene_manager_set_scene_state(
                instance->scene_manager,
                NfcSceneMfUltralightWrite,
                NfcSceneMfUltralightWriteStateCardFound);
            nfc_scene_mf_ultralight_write_setup_view(instance);
            consumed = true;
        } else if(event.event == NfcCustomEventWrongCard) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfUltralightWrongCard);
            consumed = true;
        } else if(event.event == NfcCustomEventPollerSuccess) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfUltralightWriteSuccess);
            consumed = true;
        } else if(event.event == NfcCustomEventPollerFailure) {
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfUltralightWriteFail);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_mf_ultralight_write_on_exit(void* context) {
    NfcApp* instance = context;

    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);

    scene_manager_set_scene_state(
        instance->scene_manager,
        NfcSceneMfUltralightWrite,
        NfcSceneMfUltralightWriteStateCardSearch);
    // Clear view
    popup_reset(instance->popup);

    nfc_blink_stop(instance);
}
