#include "../nfc_app_i.h"

NfcCommand
    nfc_scene_mf_ultralight_capture_pass_worker_callback(NfcGenericEvent event, void* context) {
    NfcApp* instance = context;
    MfUltralightListenerEvent* mfu_event = event.event_data;
    MfUltralightAuth* mauth = instance->mf_ul_auth;

    if(mfu_event->type == MfUltralightListenerEventTypeAuth) {
        mauth->password = mfu_event->data->password;
        view_dispatcher_send_custom_event(
            instance->view_dispatcher, MfUltralightListenerEventTypeAuth);
    }

    return NfcCommandContinue;
}

void nfc_scene_mf_ultralight_capture_pass_on_enter(void* context) {
    NfcApp* instance = context;

    // Setup view
    widget_add_string_multiline_element(
        instance->widget,
        54,
        30,
        AlignLeft,
        AlignCenter,
        FontPrimary,
        "Touch the\nreader to get\npassword...");
    widget_add_icon_element(instance->widget, 0, 15, &I_Modern_reader_18x34);
    widget_add_icon_element(instance->widget, 20, 12, &I_Move_flipper_26x39);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewWidget);

    // Start worker
    const MfUltralightData* data =
        nfc_device_get_data(instance->nfc_device, NfcProtocolMfUltralight);
    instance->listener = nfc_listener_alloc(instance->nfc, NfcProtocolMfUltralight, data);
    nfc_listener_start(
        instance->listener, nfc_scene_mf_ultralight_capture_pass_worker_callback, instance);

    nfc_blink_read_start(instance);
}

bool nfc_scene_mf_ultralight_capture_pass_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == MfUltralightListenerEventTypeAuth) {
            notification_message(instance->notifications, &sequence_success);
            scene_manager_next_scene(instance->scene_manager, NfcSceneMfUltralightUnlockWarn);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_mf_ultralight_capture_pass_on_exit(void* context) {
    NfcApp* instance = context;

    // Clear view
    nfc_listener_stop(instance->listener);
    nfc_listener_free(instance->listener);
    widget_reset(instance->widget);

    nfc_blink_stop(instance);
}
