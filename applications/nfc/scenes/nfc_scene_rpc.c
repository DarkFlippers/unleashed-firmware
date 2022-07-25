#include "../nfc_i.h"

void nfc_scene_rpc_on_enter(void* context) {
    Nfc* nfc = context;
    Popup* popup = nfc->popup;

    popup_set_header(popup, "NFC", 82, 28, AlignCenter, AlignBottom);
    popup_set_text(popup, "RPC mode", 82, 32, AlignCenter, AlignTop);

    popup_set_icon(popup, 2, 14, &I_Warning_30x23); // TODO: icon

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);

    notification_message(nfc->notifications, &sequence_display_backlight_on);
}

bool nfc_scene_rpc_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    Popup* popup = nfc->popup;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == NfcCustomEventViewExit) {
            view_dispatcher_stop(nfc->view_dispatcher);
            nfc_blink_stop(nfc);
        } else if(event.event == NfcCustomEventRpcLoad) {
            nfc_blink_start(nfc);

            nfc_text_store_set(nfc, "emulating\n%s", nfc->dev->dev_name);
            popup_set_text(popup, nfc->text_store, 82, 32, AlignCenter, AlignTop);
        }
    }
    return consumed;
}

void nfc_scene_rpc_on_exit(void* context) {
    Nfc* nfc = context;
    Popup* popup = nfc->popup;

    nfc_rpc_exit_callback(nfc);
    nfc_blink_stop(nfc);

    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
}
