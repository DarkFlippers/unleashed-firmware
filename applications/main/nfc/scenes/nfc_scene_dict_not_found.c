#include "../nfc_i.h"

void nfc_scene_dict_not_found_popup_callback(void* context) {
    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
}

void nfc_scene_dict_not_found_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup view
    Popup* popup = nfc->popup;
    popup_set_text(
        popup,
        "Function requires\nan SD card with\nfresh databases.",
        82,
        24,
        AlignCenter,
        AlignCenter);
    popup_set_icon(popup, 6, 10, &I_SDQuestion_35x43);
    popup_set_timeout(popup, 2500);
    popup_set_context(popup, nfc);
    popup_set_callback(popup, nfc_scene_dict_not_found_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
}

bool nfc_scene_dict_not_found_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            if(scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneMfClassicKeys)) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneMfClassicKeys);
            } else if(scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneExtraActions)) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneExtraActions);
            } else {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneStart);
            }
        }
    }
    return consumed;
}

void nfc_scene_dict_not_found_on_exit(void* context) {
    Nfc* nfc = context;
    popup_reset(nfc->popup);
}
