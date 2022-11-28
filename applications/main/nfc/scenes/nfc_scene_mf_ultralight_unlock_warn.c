#include "../nfc_i.h"
#include <dolphin/dolphin.h>

void nfc_scene_mf_ultralight_unlock_warn_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_mf_ultralight_unlock_warn_on_enter(void* context) {
    Nfc* nfc = context;
    DialogEx* dialog_ex = nfc->dialog_ex;
    MfUltralightAuthMethod auth_method = nfc->dev->dev_data.mf_ul_data.auth_method;

    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_mf_ultralight_unlock_warn_dialog_callback);

    if(auth_method == MfUltralightAuthMethodManual || auth_method == MfUltralightAuthMethodAuto) {
        // Build dialog text
        MfUltralightAuth* auth = &nfc->dev->dev_data.mf_ul_auth;
        FuriString* password_str =
            furi_string_alloc_set_str("Try to unlock the card with\npassword: ");
        for(size_t i = 0; i < sizeof(auth->pwd.raw); ++i) {
            furi_string_cat_printf(password_str, "%02X ", nfc->byte_input_store[i]);
        }
        furi_string_cat_str(password_str, "?\nCaution, a wrong password\ncan block the card!");
        nfc_text_store_set(nfc, furi_string_get_cstr(password_str));
        furi_string_free(password_str);

        dialog_ex_set_header(
            dialog_ex,
            auth_method == MfUltralightAuthMethodAuto ? "Password captured!" : "Risky function!",
            64,
            0,
            AlignCenter,
            AlignTop);
        dialog_ex_set_text(dialog_ex, nfc->text_store, 64, 12, AlignCenter, AlignTop);
        dialog_ex_set_left_button_text(dialog_ex, "Cancel");
        dialog_ex_set_right_button_text(dialog_ex, "Continue");

        if(auth_method == MfUltralightAuthMethodAuto)
            notification_message(nfc->notifications, &sequence_set_green_255);
    } else {
        dialog_ex_set_header(dialog_ex, "Risky function!", 64, 4, AlignCenter, AlignTop);
        dialog_ex_set_text(
            dialog_ex, "Wrong password\ncan block your\ncard.", 4, 18, AlignLeft, AlignTop);
        dialog_ex_set_icon(dialog_ex, 73, 20, &I_DolphinCommon_56x48);
        dialog_ex_set_center_button_text(dialog_ex, "OK");
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_mf_ultralight_unlock_warn_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;

    bool consumed = false;

    MfUltralightAuthMethod auth_method = nfc->dev->dev_data.mf_ul_data.auth_method;
    if(auth_method == MfUltralightAuthMethodManual || auth_method == MfUltralightAuthMethodAuto) {
        if(event.type == SceneManagerEventTypeCustom) {
            if(event.event == DialogExResultRight) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightReadAuth);
                DOLPHIN_DEED(DolphinDeedNfcRead);
                consumed = true;
            } else if(event.event == DialogExResultLeft) {
                if(auth_method == MfUltralightAuthMethodAuto) {
                    consumed = scene_manager_search_and_switch_to_previous_scene(
                        nfc->scene_manager, NfcSceneMfUltralightUnlockMenu);
                } else {
                    consumed = scene_manager_previous_scene(nfc->scene_manager);
                }
            }
        } else if(event.type == SceneManagerEventTypeBack) {
            // Cannot press back
            consumed = true;
        }
    } else {
        if(event.type == SceneManagerEventTypeCustom) {
            if(event.event == DialogExResultCenter) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightReadAuth);
                DOLPHIN_DEED(DolphinDeedNfcRead);
                consumed = true;
            }
        }
    }

    return consumed;
}

void nfc_scene_mf_ultralight_unlock_warn_on_exit(void* context) {
    Nfc* nfc = context;

    dialog_ex_reset(nfc->dialog_ex);
    nfc_text_store_clear(nfc);

    notification_message_block(nfc->notifications, &sequence_reset_green);
}
