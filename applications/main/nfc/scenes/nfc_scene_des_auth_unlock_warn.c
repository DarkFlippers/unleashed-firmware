#include "../nfc_app_i.h"

void nfc_scene_des_auth_unlock_warn_dialog_callback(DialogExResult result, void* context) {
    NfcApp* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_des_auth_unlock_warn_on_enter(void* context) {
    NfcApp* nfc = context;

    const char* message = "Risky Action!";
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_des_auth_unlock_warn_dialog_callback);

    dialog_ex_set_header(dialog_ex, message, 64, 0, AlignCenter, AlignTop);

    FuriString* str = furi_string_alloc();
    furi_string_cat_printf(str, "Unlock with key: ");

    NfcProtocol protocol = nfc_device_get_protocol(nfc->nfc_device);
    uint8_t* key = (protocol == NfcProtocolFelica) ? nfc->felica_auth->card_key.data :
                                                     nfc->mf_ul_auth->tdes_key.data;

    for(uint8_t i = 0; i < FELICA_DATA_BLOCK_SIZE; i++)
        furi_string_cat_printf(str, "%02X ", key[i]);
    furi_string_cat_printf(str, "?");

    nfc_text_store_set(nfc, furi_string_get_cstr(str));
    furi_string_free(str);

    dialog_ex_set_text(dialog_ex, nfc->text_store, 0, 12, AlignLeft, AlignTop);

    dialog_ex_set_left_button_text(dialog_ex, "Cancel");
    dialog_ex_set_right_button_text(dialog_ex, "Unlock");

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_des_auth_unlock_warn_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    UNUSED(event);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultRight) {
            nfc->felica_auth->skip_auth = false;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRead);
            consumed = true;
        } else if(event.event == DialogExResultLeft) {
            scene_manager_previous_scene(nfc->scene_manager);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_des_auth_unlock_warn_on_exit(void* context) {
    NfcApp* nfc = context;

    dialog_ex_reset(nfc->dialog_ex);
    nfc_text_store_clear(nfc);
}
