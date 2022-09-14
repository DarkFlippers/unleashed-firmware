#include "../nfc_i.h"

void nfc_scene_field_on_enter(void* context) {
    Nfc* nfc = context;

    furi_hal_nfc_field_on();

    Popup* popup = nfc->popup;
    popup_set_header(
        popup,
        "Field is on\nDon't leave device\nin this mode for too long.",
        64,
        11,
        AlignCenter,
        AlignTop);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);

    notification_internal_message(nfc->notifications, &sequence_set_blue_255);
}

bool nfc_scene_field_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void nfc_scene_field_on_exit(void* context) {
    Nfc* nfc = context;

    furi_hal_nfc_field_off();
    notification_internal_message(nfc->notifications, &sequence_reset_blue);
    popup_reset(nfc->popup);
}
