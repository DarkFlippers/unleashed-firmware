#include "../nfc_i.h"

void nfc_scene_rpc_on_enter(void* context) {
    Nfc* nfc = context;
    Widget* widget = nfc->widget;

    widget_add_text_box_element(
        widget, 0, 0, 128, 28, AlignCenter, AlignCenter, "RPC mode", false);

    notification_message(nfc->notifications, &sequence_display_backlight_on);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == NfcCustomEventViewExit) {
            view_dispatcher_stop(nfc->view_dispatcher);
        }
    }
    return consumed;
}

void nfc_scene_rpc_on_exit(void* context) {
    Nfc* nfc = context;

    nfc_rpc_exit_callback(nfc);

    widget_reset(nfc->widget);
}
