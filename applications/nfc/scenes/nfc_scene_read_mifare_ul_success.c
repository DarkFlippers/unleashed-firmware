#include "../nfc_i.h"

#define NFC_SCENE_READ_SUCCESS_SHIFT "              "
#define NFC_SCENE_READ_MF_UL_CUSTOM_EVENT (0UL)

enum {
    ReadMifareUlStateShowUID,
    ReadMifareUlStateShowData,
};

void nfc_scene_read_mifare_ul_success_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_read_mifare_ul_success_text_box_callback(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, NFC_SCENE_READ_MF_UL_CUSTOM_EVENT);
}

void nfc_scene_read_mifare_ul_success_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Send notification
    notification_message(nfc->notifications, &sequence_success);

    // Setup dialog view
    NfcDeviceCommonData* data = &nfc->dev->dev_data.nfc_data;
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "More");
    dialog_ex_set_center_button_text(dialog_ex, "Data");
    dialog_ex_set_header(dialog_ex, "Mifare Ultralight", 22, 8, AlignLeft, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 8, 13, &I_Medium_chip_22x21);
    // Display UID
    nfc_text_store_set(
        nfc,
        NFC_SCENE_READ_SUCCESS_SHIFT "ATQA: %02X%02X\n" NFC_SCENE_READ_SUCCESS_SHIFT
                                     "SAK: %02X\nUID: %02X %02X %02X %02X %02X %02X %02X",
        data->atqa[0],
        data->atqa[1],
        data->sak,
        data->uid[0],
        data->uid[1],
        data->uid[2],
        data->uid[3],
        data->uid[4],
        data->uid[5],
        data->uid[6]);
    dialog_ex_set_text(dialog_ex, nfc->text_store, 8, 16, AlignLeft, AlignTop);
    dialog_ex_set_context(dialog_ex, nfc);
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_read_mifare_ul_success_dialog_callback);

    // Setup TextBox view
    MifareUlData* mf_ul_data = &nfc->dev->dev_data.mf_ul_data;
    TextBox* text_box = nfc->text_box;
    text_box_set_context(text_box, nfc);
    text_box_set_exit_callback(text_box, nfc_scene_read_mifare_ul_success_text_box_callback);
    text_box_set_font(text_box, TextBoxFontHex);
    for(uint16_t i = 0; i < mf_ul_data->data_size; i += 2) {
        if(!(i % 8) && i) {
            string_push_back(nfc->text_box_store, '\n');
        }
        string_cat_printf(
            nfc->text_box_store, "%02X%02X ", mf_ul_data->data[i], mf_ul_data->data[i + 1]);
    }
    text_box_set_text(text_box, string_get_cstr(nfc->text_box_store));

    scene_manager_set_scene_state(
        nfc->scene_manager, NfcSceneReadMifareUlSuccess, ReadMifareUlStateShowUID);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_read_mifare_ul_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if((scene_manager_get_scene_state(nfc->scene_manager, NfcSceneReadMifareUlSuccess) ==
            ReadMifareUlStateShowUID) &&
           (event.event == DialogExResultLeft)) {
            scene_manager_previous_scene(nfc->scene_manager);
            return true;
        } else if(
            (scene_manager_get_scene_state(nfc->scene_manager, NfcSceneReadMifareUlSuccess) ==
             ReadMifareUlStateShowUID) &&
            (event.event == DialogExResultRight)) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMifareUlMenu);
            return true;
        } else if(
            (scene_manager_get_scene_state(nfc->scene_manager, NfcSceneReadMifareUlSuccess) ==
             ReadMifareUlStateShowUID) &&
            (event.event == DialogExResultCenter)) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneReadMifareUlSuccess, ReadMifareUlStateShowData);
            return true;
        } else if(
            (scene_manager_get_scene_state(nfc->scene_manager, NfcSceneReadMifareUlSuccess) ==
             ReadMifareUlStateShowData) &&
            (event.event == NFC_SCENE_READ_MF_UL_CUSTOM_EVENT)) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneReadMifareUlSuccess, ReadMifareUlStateShowUID);
            return true;
        }
    }
    return false;
}

void nfc_scene_read_mifare_ul_success_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clean dialog
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_header(dialog_ex, NULL, 0, 0, AlignCenter, AlignCenter);
    dialog_ex_set_text(dialog_ex, NULL, 0, 0, AlignCenter, AlignTop);
    dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
    dialog_ex_set_left_button_text(dialog_ex, NULL);
    dialog_ex_set_right_button_text(dialog_ex, NULL);
    dialog_ex_set_center_button_text(dialog_ex, NULL);
    dialog_ex_set_result_callback(dialog_ex, NULL);
    dialog_ex_set_context(dialog_ex, NULL);

    // Clean TextBox
    TextBox* text_box = nfc->text_box;
    text_box_clean(text_box);
    string_reset(nfc->text_box_store);
}
