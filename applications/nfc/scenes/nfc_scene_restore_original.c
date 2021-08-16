#include "../nfc_i.h"

#define SCENE_RESTORE_ORIGINAL_CUSTOM_EVENT (0UL)

void nfc_scene_restore_original_popup_callback(void* context) {
    Nfc* nfc = (Nfc*)context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, SCENE_RESTORE_ORIGINAL_CUSTOM_EVENT);
}

const void nfc_scene_restore_original_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

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

const bool nfc_scene_restore_original_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_RESTORE_ORIGINAL_CUSTOM_EVENT) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        }
    }
    return consumed;
}

const void nfc_scene_restore_original_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear view
    Popup* popup = nfc->popup;
    popup_set_header(popup, NULL, 0, 0, AlignCenter, AlignBottom);
    popup_set_text(popup, NULL, 0, 0, AlignCenter, AlignTop);
    popup_set_icon(popup, 0, 0, NULL);
    popup_set_callback(popup, NULL);
    popup_set_context(popup, NULL);
    popup_set_timeout(popup, 0);
    popup_disable_timeout(popup);
}
