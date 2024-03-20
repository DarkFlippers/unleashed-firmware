#include "../nfc_app_i.h"

void nfc_scene_mf_classic_keys_warn_duplicate_popup_callback(void* context) {
    NfcApp* instance = context;

    view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventViewExit);
}

void nfc_scene_mf_classic_keys_warn_duplicate_on_enter(void* context) {
    NfcApp* instance = context;

    // Setup view
    Popup* popup = instance->popup;
    popup_set_icon(popup, 83, 22, &I_WarningDolphinFlip_45x42);
    popup_set_header(popup, "Key Already Exists!", 64, 3, AlignCenter, AlignTop);
    popup_set_text(
        popup,
        "Please enter a\n"
        "different key.",
        4,
        24,
        AlignLeft,
        AlignTop);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, instance);
    popup_set_callback(popup, nfc_scene_mf_classic_keys_warn_duplicate_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(instance->view_dispatcher, NfcViewPopup);
}

bool nfc_scene_mf_classic_keys_warn_duplicate_on_event(void* context, SceneManagerEvent event) {
    NfcApp* instance = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, NfcSceneMfClassicKeysAdd);
        }
    }

    return consumed;
}

void nfc_scene_mf_classic_keys_warn_duplicate_on_exit(void* context) {
    NfcApp* instance = context;

    popup_reset(instance->popup);
}
