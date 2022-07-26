#include "../nfc_i.h"
#include <dolphin/dolphin.h>

#define NFC_SCENE_READ_SUCCESS_SHIFT "              "

enum {
    ReadMifareUlStateShowUID,
    ReadMifareUlStateShowData,
};

void nfc_scene_mf_ultralight_read_success_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_mf_ultralight_read_success_on_enter(void* context) {
    Nfc* nfc = context;
    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup dialog view
    FuriHalNfcDevData* data = &nfc->dev->dev_data.nfc_data;
    MfUltralightData* mf_ul_data = &nfc->dev->dev_data.mf_ul_data;
    DialogEx* dialog_ex = nfc->dialog_ex;
    dialog_ex_set_left_button_text(dialog_ex, "Retry");
    dialog_ex_set_right_button_text(dialog_ex, "More");
    dialog_ex_set_center_button_text(dialog_ex, "Data");
    dialog_ex_set_header(
        dialog_ex, nfc_mf_ul_type(mf_ul_data->type, true), 64, 8, AlignCenter, AlignCenter);
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
    dialog_ex_set_result_callback(dialog_ex, nfc_scene_mf_ultralight_read_success_dialog_callback);

    // Setup TextBox view
    TextBox* text_box = nfc->text_box;
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
        nfc->scene_manager, NfcSceneMfUltralightReadSuccess, ReadMifareUlStateShowUID);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
}

bool nfc_scene_mf_ultralight_read_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;
    uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfUltralightReadSuccess);

    if(event.type == SceneManagerEventTypeCustom) {
        if(state == ReadMifareUlStateShowUID && event.event == DialogExResultLeft) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRetryConfirm);
            consumed = true;
        } else if(state == ReadMifareUlStateShowUID && event.event == DialogExResultRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightMenu);
            consumed = true;
        } else if(state == ReadMifareUlStateShowUID && event.event == DialogExResultCenter) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfUltralightReadSuccess, ReadMifareUlStateShowData);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state == ReadMifareUlStateShowData) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfUltralightReadSuccess, ReadMifareUlStateShowUID);
            consumed = true;
        } else {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneExitConfirm);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_mf_ultralight_read_success_on_exit(void* context) {
    Nfc* nfc = context;

    // Clean views
    dialog_ex_reset(nfc->dialog_ex);
    text_box_reset(nfc->text_box);
    string_reset(nfc->text_box_store);
}
