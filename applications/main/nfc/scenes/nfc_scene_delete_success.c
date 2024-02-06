#include "../nfc_app_i.h"

void nfc_scene_delete_success_popup_callback(void* context) {
    NfcApp* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
}

void nfc_scene_delete_success_on_enter(void* context) {
    NfcApp* nfc = context;

    // Setup view
    Popup* popup = nfc->popup;
    popup_set_icon(popup, 0, 2, &I_DolphinMafia_119x62);
    popup_set_header(popup, "Deleted", 80, 19, AlignLeft, AlignBottom);
    popup_set_timeout(popup, 1500);
    popup_set_context(popup, nfc);
    popup_set_callback(popup, nfc_scene_delete_success_popup_callback);
    popup_enable_timeout(popup);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewPopup);
}

bool nfc_scene_delete_success_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventViewExit) {
            if(scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneMfClassicKeys)) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneMfClassicKeys);
            } else {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneFileSelect);

                if(!consumed) {
                    scene_manager_stop(nfc->scene_manager);
                    view_dispatcher_stop(nfc->view_dispatcher);
                }
            }
        }
    }
    return consumed;
}

void nfc_scene_delete_success_on_exit(void* context) {
    NfcApp* nfc = context;

    // Clear view
    popup_reset(nfc->popup);
}
