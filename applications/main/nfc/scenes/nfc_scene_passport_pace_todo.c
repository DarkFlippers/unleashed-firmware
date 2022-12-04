#include "../nfc_i.h"

void nfc_scene_passport_pace_todo_popup_callback(void* context) {
    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
}

void nfc_scene_passport_pace_todo_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup view
    Popup* popup = nfc->popup;
    popup_set_icon(popup, 64, 16, &I_DolphinCommon_56x48);
    popup_set_header(popup, "PACE not yet implemented", 4, 4, AlignLeft, AlignTop);
    popup_set_timeout(popup, 2000);
    popup_set_context(popup, nfc);
    popup_set_callback(popup, nfc_scene_passport_pace_todo_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
}

bool nfc_scene_passport_pace_todo_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                nfc->scene_manager, NfcScenePassportAuth);
        }
    }
    return consumed;
}

void nfc_scene_passport_pace_todo_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    popup_reset(nfc->popup);
}
