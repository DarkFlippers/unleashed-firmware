#include "../nfc_app_i.h"

void nfc_scene_retry_confirm_dialog_callback(DialogExResult result, void* context) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_retry_confirm_on_enter(void* context) {
    NfcApp* nfc = context;
    DialogEx* dialog_ex = nfc->dialog_ex;

    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "Stay");
    dialog_ex_set_header(dialog_ex, "Retry Reading?", 64, 0, AlignCenter, AlignTop);
    dialog_ex_set_text(dialog_ex, "All unsaved data will be lost", 64, 12, AlignCenter, AlignTop);
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_retry_confirm_dialog_callback);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_retry_confirm_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultRight) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else if(event.event == DialogExResultLeft) {
            if(scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneSlixUnlock)) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneSlixUnlock);
            } else if(
                scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneMfClassicDictAttack) &&
                (scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneReadMenu) ||
                 scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneSavedMenu))) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneMfClassicDictAttack);
            } else if(scene_manager_has_previous_scene(
                          nfc->scene_manager, NfcSceneMfUltralightUnlockWarn)) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneMfUltralightUnlockMenu);
            } else if(scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneDetect)) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneDetect);
            } else if(scene_manager_has_previous_scene(nfc->scene_manager, NfcSceneRead)) {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneRead);
            }
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

void nfc_scene_retry_confirm_on_exit(void* context) {
    NfcApp* nfc = context;

    // Clean view
    dialog_ex_reset(nfc->dialog_ex);
}
