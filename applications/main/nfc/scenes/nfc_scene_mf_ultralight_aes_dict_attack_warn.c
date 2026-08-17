#include "../nfc_app_i.h"

// UL-AES has an AUTHLIM, failed keys can lock the card permanently

void nfc_scene_mf_ultralight_aes_dict_attack_warn_dialog_callback(
    DialogExResult result,
    void* context) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_mf_ultralight_aes_dict_attack_warn_on_enter(void* context) {
    NfcApp* nfc = context;
    DialogEx* dialog_ex = nfc->dialog_ex;

    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(
        dialog_ex, nfc_scene_mf_ultralight_aes_dict_attack_warn_dialog_callback);

    dialog_ex_set_header(dialog_ex, "Risky action!", 64, 4, AlignCenter, AlignTop);
    dialog_ex_set_text(dialog_ex, "Wrong keys can\nblock this card", 4, 18, AlignLeft, AlignTop);
    dialog_ex_set_icon(dialog_ex, 83, 22, &I_WarningDolphinFlip_45x42);
    dialog_ex_set_left_button_text(dialog_ex, "Cancel");
    dialog_ex_set_right_button_text(dialog_ex, "Continue");

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_mf_ultralight_aes_dict_attack_warn_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;

    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightAesDictAttack);
            consumed = true;
        } else if(event.event == DialogExResultLeft) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        }
    }

    return consumed;
}

void nfc_scene_mf_ultralight_aes_dict_attack_warn_on_exit(void* context) {
    NfcApp* nfc = context;

    dialog_ex_reset(nfc->dialog_ex);
}
