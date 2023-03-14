#include "../nfc_magic_i.h"

void nfc_magic_scene_success_popup_callback(void* context) {
    NfcMagic* nfc_magic = context;
    view_dispatcher_send_custom_event(nfc_magic->view_dispatcher, NfcMagicCustomEventViewExit);
}

void nfc_magic_scene_success_on_enter(void* context) {
    NfcMagic* nfc_magic = context;

    notification_message(nfc_magic->notifications, &sequence_success);

    Popup* popup = nfc_magic->popup;
    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Success!", 10, 20, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, nfc_magic);
    popup_set_callback(popup, nfc_magic_scene_success_popup_callback);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(nfc_magic->view_dispatcher, NfcMagicViewPopup);
}

bool nfc_magic_scene_success_on_event(void* context, SceneManagerEvent event) {
    NfcMagic* nfc_magic = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcMagicCustomEventViewExit) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                nfc_magic->scene_manager, NfcMagicSceneStart);
        }
    }
    return consumed;
}

void nfc_magic_scene_success_on_exit(void* context) {
    NfcMagic* nfc_magic = context;

    // Clear view
    popup_reset(nfc_magic->popup);
}
