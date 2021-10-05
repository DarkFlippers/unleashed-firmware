#include "../nfc_i.h"
#include "../helpers/nfc_emv_parser.h"

#define NFC_SCENE_READ_SUCCESS_SHIFT "              "

void nfc_scene_read_emv_app_success_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_read_emv_app_success_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup view
    NfcDeviceCommonData* nfc_data = &nfc->dev.dev_data.nfc_data;
    NfcEmvData* emv_data = &nfc->dev.dev_data.emv_data;
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "Run app");
    dialog_ex_set_header(dialog_ex, "Found EMV App", 36, 8, AlignLeft, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 8, 13, &I_Medium_chip_22x21);
    // Display UID and AID
    string_t aid;
    string_init(aid);
    bool aid_found = nfc_emv_parser_get_aid_name(emv_data->aid, emv_data->aid_len, aid);
    if(!aid_found) {
        for(uint8_t i = 0; i < emv_data->aid_len; i++) {
            string_cat_printf(aid, "%02X", emv_data->aid[i]);
        }
    }
    nfc_text_store_set(
        nfc,
        NFC_SCENE_READ_SUCCESS_SHIFT "UID: %02X %02X %02X %02X \n" NFC_SCENE_READ_SUCCESS_SHIFT
                                     "Application:\n%s",
        nfc_data->uid[0],
        nfc_data->uid[1],
        nfc_data->uid[2],
        nfc_data->uid[3],
        string_get_cstr(aid));
    string_clear(aid);
    dialog_ex_set_text(dialog_ex, nfc->text_store, 8, 16, AlignLeft, AlignTop);
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_read_emv_app_success_dialog_callback);

    // Send notification
    if(scene_manager_get_scene_state(nfc->scene_manager, NfcSceneReadEmvAppSuccess) ==
       NFC_SEND_NOTIFICATION_TRUE) {
        notification_message(nfc->notifications, &sequence_success);
        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneReadEmvAppSuccess, NFC_SEND_NOTIFICATION_FALSE);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_read_emv_app_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            return scene_manager_previous_scene(nfc->scene_manager);
        } else if(event.event == DialogExResultRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRunEmvAppConfirm);
            return true;
        }
    }
    return false;
}

void nfc_scene_read_emv_app_success_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_header(dialog_ex, NULL, 0, 0, AlignCenter, AlignCenter);
    dialog_ex_set_text(dialog_ex, NULL, 0, 0, AlignCenter, AlignTop);
    dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
    dialog_ex_set_left_button_text(dialog_ex, NULL);
    dialog_ex_set_right_button_text(dialog_ex, NULL);
    dialog_ex_set_result_callback(dialog_ex, NULL);
    dialog_ex_set_context(dialog_ex, NULL);
}
