#include "../nfc_i.h"

void nfc_scene_restore_original_popup_callback(void* context) {
    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
}

void nfc_scene_restore_original_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup view
    Popup* popup = nfc->popup;
    popup_set_icon(popup, 32, 5, &I_DolphinNice_96x59);
    popup_set_header(popup, "Original file\nrestored", 13, 22, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, nfc);
    popup_set_callback(popup, nfc_scene_restore_original_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
}

bool nfc_scene_restore_original_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        }
    }
    return consumed;
}

void nfc_scene_restore_original_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    popup_reset(nfc->popup);
}
