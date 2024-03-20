#include "../nfc_app_i.h"

#include <nfc/protocols/slix/slix_poller.h>

NfcCommand nfc_scene_slix_unlock_worker_callback(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolSlix);

    NfcCommand command = NfcCommandContinue;

    NfcApp* instance = context;
    SlixPollerEvent* slix_event = event.event_data;
    if(slix_event->type == SlixPollerEventTypePrivacyUnlockRequest) {
        SlixPassword pwd = 0;
        bool get_password_success = slix_unlock_get_next_password(instance->slix_unlock, &pwd);
        slix_event->data->privacy_password.password = pwd;
        slix_event->data->privacy_password.password_set = get_password_success;
    } else if(slix_event->type == SlixPollerEventTypeError) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
    } else if(slix_event->type == SlixPollerEventTypeReady) {
        nfc_device_set_data(
            instance->nfc_device, NfcProtocolSlix, nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        command = NfcCommandStop;
    }

    return command;
}

void nfc_scene_slix_unlock_on_enter(void* context) {
    NfcApp* instance = context;

    popup_set_icon(instance->popup, 0, 8, &I_NFC_manual_60x50);
    popup_set_header(instance->popup, "Unlocking", 97, 15, AlignCenter, AlignTop);
    popup_set_text(
        instance->popup, "Hold card next\nto Flipper's back", 94, 27, AlignCenter, AlignTop);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);

    instance->poller = nfc_poller_alloc(instance->nfc, NfcProtocolSlix);
    nfc_poller_start(instance->poller, nfc_scene_slix_unlock_worker_callback, instance);
}

bool nfc_scene_slix_unlock_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    UNUSED(instance);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventPollerFailure) {
            consumed = true;
        } else if(event.event == NfcCustomEventPollerSuccess) {
            notification_message(instance->notifications, &sequence_success);
            scene_manager_next_scene(instance->scene_manager, NfcSceneSlixUnlockSuccess);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            instance->scene_manager, NfcSceneSlixUnlockMenu);
    }

    return consumed;
}

void nfc_scene_slix_unlock_on_exit(void* context) {
    NfcApp* instance = context;

    nfc_poller_stop(instance->poller);
    nfc_poller_free(instance->poller);

    popup_reset(instance->popup);
}
