#include "../nfc_i.h"

void nfc_scene_run_emv_app_confirm_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_run_emv_app_confirm_on_enter(void* context) {
    Nfc* nfc = context;

    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_left_button_text(dialog_ex, "Back");
    dialog_ex_set_right_button_text(dialog_ex, "Run");
    dialog_ex_set_header(dialog_ex, "Run EMV app?", 64, 8, AlignCenter, AlignCenter);
    dialog_ex_set_text(
        dialog_ex,
        "It will try to run card's app\nand detect unencrypted\ndata",
        64,
        18,
        AlignCenter,
        AlignTop);
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_run_emv_app_confirm_dialog_callback);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_run_emv_app_confirm_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else if(event.event == DialogExResultRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneReadEmvData);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_run_emv_app_confirm_on_exit(void* context) {
    Nfc* nfc = context;

    // Clean view
    dialog_ex_reset(nfc->dialog_ex);
}
