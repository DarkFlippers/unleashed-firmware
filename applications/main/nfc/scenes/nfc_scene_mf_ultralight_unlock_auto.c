#include "../nfc_i.h"

bool nfc_scene_mf_ultralight_unlock_auto_worker_callback(NfcWorkerEvent event, void* context) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, event);
    return true;
}

void nfc_scene_mf_ultralight_unlock_auto_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup view
    popup_set_text(nfc->popup, "Touch the reader", 44, 31, AlignLeft, AlignCenter);
    popup_set_icon(nfc->popup, 0, 16, &I_Tap_reader_36x38);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);

    // Start worker
    nfc_worker_start(
        nfc->worker,
        NfcWorkerStateMfUltralightEmulate,
        &nfc->dev->dev_data,
        nfc_scene_mf_ultralight_unlock_auto_worker_callback,
        nfc);

    nfc_blink_emulate_start(nfc);
}

bool nfc_scene_mf_ultralight_unlock_auto_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if((event.event == NfcWorkerEventMfUltralightPwdAuth)) {
            MfUltralightAuth* auth = &nfc->dev->dev_data.mf_ul_auth;
            memcpy(nfc->byte_input_store, auth->pwd.raw, sizeof(auth->pwd.raw));
            nfc->dev->dev_data.mf_ul_data.auth_method = MfUltralightAuthMethodManual;
            nfc_worker_stop(nfc->worker);
            notification_message(nfc->notifications, &sequence_success);
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightUnlockWarn);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_mf_ultralight_unlock_auto_on_exit(void* context) {
    Nfc* nfc = context;

    // Stop worker
    nfc_worker_stop(nfc->worker);
    // Clear view
    popup_reset(nfc->popup);

    nfc_blink_stop(nfc);
}
