#include "../nfc_rfid_detector_app_i.h"
#include "../views/nfc_rfid_detector_view_field_presence.h"

void nfc_rfid_detector_scene_field_presence_callback(
    NfcRfidDetectorCustomEvent event,
    void* context) {
    furi_assert(context);
    NfcRfidDetectorApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, event);
}

static const NotificationSequence notification_app_display_on = {

    &message_display_backlight_on,
    NULL,
};

static void nfc_rfid_detector_scene_field_presence_update(void* context) {
    furi_assert(context);
    NfcRfidDetectorApp* app = context;

    uint32_t frequency = 0;
    bool nfc_field = nfc_rfid_detector_app_field_presence_is_nfc(app);
    bool rfid_field = nfc_rfid_detector_app_field_presence_is_rfid(app, &frequency);

    if(nfc_field || rfid_field)
        notification_message(app->notifications, &notification_app_display_on);

    nfc_rfid_detector_view_field_presence_update(
        app->nfc_rfid_detector_field_presence, nfc_field, rfid_field, frequency);
}

void nfc_rfid_detector_scene_field_presence_on_enter(void* context) {
    furi_assert(context);
    NfcRfidDetectorApp* app = context;

    // Start detection of field presence
    nfc_rfid_detector_app_field_presence_start(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, NfcRfidDetectorViewFieldPresence);
}

bool nfc_rfid_detector_scene_field_presence_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);
    NfcRfidDetectorApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        nfc_rfid_detector_scene_field_presence_update(app);
    }

    return consumed;
}

void nfc_rfid_detector_scene_field_presence_on_exit(void* context) {
    furi_assert(context);
    NfcRfidDetectorApp* app = context;
    // Stop detection of field presence
    nfc_rfid_detector_app_field_presence_stop(app);
}
