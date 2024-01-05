#include "../nfc_app_i.h"

void nfc_scene_mf_classic_update_initial_success_popup_callback(void* context) {
    NfcApp* instance = context;
    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventViewExit);
}

void nfc_scene_mf_classic_update_initial_success_on_enter(void* context) {
    NfcApp* instance = context;
    dolphin_deed(DolphinDeedNfcSave);

    notification_message(instance->notifications, &sequence_success);

    Popup* popup = instance->popup;
    popup_set_icon(popup, 48, 6, &I_DolphinDone_80x58);
    popup_set_header(popup, "Updated", 11, 20, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, instance);
    popup_set_callback(popup, nfc_scene_mf_classic_update_initial_success_popup_callback);
    popup_enable_timeout(popup);

    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);
}

bool nfc_scene_mf_classic_update_initial_success_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcSceneSavedMenu);
        }
    }
    return consumed;
}

void nfc_scene_mf_classic_update_initial_success_on_exit(void* context) {
    NfcApp* instance = context;

    // Clear view
    popup_reset(instance->popup);
}
