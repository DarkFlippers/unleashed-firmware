#include "../nfc_app_i.h"

void nfc_scene_restore_original_confirm_dialog_callback(DialogExResult result, void* context) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_restore_original_confirm_on_enter(void* context) {
    NfcApp* nfc = context;
    DialogEx* dialog_ex = nfc->dialog_ex;

    dialog_ex_set_header(dialog_ex, "Restore Card Data?", 64, 0, AlignCenter, AlignTop);
    dialog_ex_set_icon(dialog_ex, 5, 11, &I_ArrowC_1_36x36);
    dialog_ex_set_text(
        dialog_ex, "It will be returned\nto its original state.", 47, 21, AlignLeft, AlignTop);
    dialog_ex_set_left_button_text(dialog_ex, "Cancel");
    dialog_ex_set_right_button_text(dialog_ex, "Restore");
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_restore_original_confirm_dialog_callback);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_restore_original_confirm_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultRight) {
            if(nfc_delete_shadow_file(nfc)) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneRestoreOriginal);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneStart);
            }
            consumed = true;
        } else if(event.event == DialogExResultLeft) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

void nfc_scene_restore_original_confirm_on_exit(void* context) {
    NfcApp* nfc = context;

    // Clean view
    dialog_ex_reset(nfc->dialog_ex);
}
