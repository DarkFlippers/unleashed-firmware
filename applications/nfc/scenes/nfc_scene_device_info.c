#include "../nfc_i.h"

#define NFC_SCENE_DEVICE_INFO_TEXTBOX_CUSTOM_EVENT (0UL)

enum {
    NfcSceneDeviceInfoUid,
    NfcSceneDeviceInfoData,
};

void nfc_scene_device_info_widget_callback(GuiButtonType result, void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_device_info_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
}

void nfc_scene_device_info_text_box_callback(void* context) {
    Nfc* nfc = (Nfc*)context;

    view_dispatcher_send_custom_event(
        nfc->view_dispatcher, NFC_SCENE_DEVICE_INFO_TEXTBOX_CUSTOM_EVENT);
}

void nfc_scene_device_info_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup Custom Widget view
    widget_add_string_element(
        nfc->widget, 64, 6, AlignCenter, AlignTop, FontSecondary, nfc->dev.dev_name);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeLeft, "Back", nfc_scene_device_info_widget_callback, nfc);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeRight, "Data", nfc_scene_device_info_widget_callback, nfc);
    char uid_str[32];
    NfcDeviceCommomData* data = &nfc->dev.dev_data.nfc_data;
    if(data->uid_len == 4) {
        snprintf(
            uid_str,
            sizeof(uid_str),
            "UID: %02X %02X %02X %02X",
            data->uid[0],
            data->uid[1],
            data->uid[2],
            data->uid[3]);
    } else if(data->uid_len == 7) {
        snprintf(
            uid_str,
            sizeof(uid_str),
            "UID: %02X %02X %02X %02X %02X %02X %02X",
            data->uid[0],
            data->uid[1],
            data->uid[2],
            data->uid[3],
            data->uid[4],
            data->uid[5],
            data->uid[6]);
    }
    widget_add_string_element(nfc->widget, 64, 21, AlignCenter, AlignTop, FontSecondary, uid_str);

    if(data->protocol > NfcDeviceProtocolUnknown) {
        widget_add_string_element(
            nfc->widget,
            10,
            32,
            AlignLeft,
            AlignTop,
            FontSecondary,
            nfc_get_protocol(data->protocol));
    }
    // TODO change dinamically
    widget_add_string_element(nfc->widget, 118, 32, AlignRight, AlignTop, FontSecondary, "NFC-A");
    char sak_str[16];
    snprintf(sak_str, sizeof(sak_str), "SAK: %02X", data->sak);
    widget_add_string_element(nfc->widget, 10, 42, AlignLeft, AlignTop, FontSecondary, sak_str);
    char atqa_str[16];
    snprintf(atqa_str, sizeof(atqa_str), "ATQA: %02X%02X", data->atqa[0], data->atqa[1]);
    widget_add_string_element(nfc->widget, 118, 42, AlignRight, AlignTop, FontSecondary, atqa_str);

    // Setup Data View
    if(nfc->dev.format == NfcDeviceSaveFormatUid) {
        DialogEx* dialog_ex = nfc->dialog_ex;
        dialog_ex_set_left_button_text(dialog_ex, "Back");
        dialog_ex_set_text(dialog_ex, "No data", 64, 32, AlignCenter, AlignCenter);
        dialog_ex_set_context(dialog_ex, nfc);
        dialog_ex_set_result_callback(dialog_ex, nfc_scene_device_info_dialog_callback);
    } else if(nfc->dev.format == NfcDeviceSaveFormatMifareUl) {
        MifareUlData* mf_ul_data = (MifareUlData*)&nfc->dev.dev_data.mf_ul_data;
        TextBox* text_box = nfc->text_box;
        text_box_set_context(text_box, nfc);
        text_box_set_exit_callback(text_box, nfc_scene_device_info_text_box_callback);
        text_box_set_font(text_box, TextBoxFontHex);
        for(uint16_t i = 0; i < mf_ul_data->data_size; i += 2) {
            if(!(i % 8) && i) {
                string_push_back(nfc->text_box_store, '\n');
            }
            string_cat_printf(
                nfc->text_box_store, "%02X%02X ", mf_ul_data->data[i], mf_ul_data->data[i + 1]);
        }
        text_box_set_text(text_box, string_get_cstr(nfc->text_box_store));
    } else if(nfc->dev.format == NfcDeviceSaveFormatBankCard) {
        NfcEmvData* emv_data = &nfc->dev.dev_data.emv_data;
        BankCard* bank_card = nfc->bank_card;
        bank_card_set_name(bank_card, emv_data->name);
        bank_card_set_number(bank_card, emv_data->number);
        if(!strcmp(emv_data->name, "")) {
            bank_card_set_cardholder_name(bank_card, emv_data->cardholder);
        }
        if(emv_data->exp_mon) {
            bank_card_set_exp_date(bank_card, emv_data->exp_mon, emv_data->exp_year);
        }
    }
    scene_manager_set_scene_state(nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoUid);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

const bool nfc_scene_device_info_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;
    bool consumed = false;
    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneDeviceInfo);

    if(event.type == SceneManagerEventTypeCustom) {
        if((state == NfcSceneDeviceInfoUid) && (event.event == GuiButtonTypeLeft)) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else if((state == NfcSceneDeviceInfoUid) && (event.event == GuiButtonTypeRight)) {
            if(nfc->dev.format == NfcDeviceSaveFormatUid) {
                scene_manager_set_scene_state(
                    nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoData);
                view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
                consumed = true;
            } else if(nfc->dev.format == NfcDeviceSaveFormatMifareUl) {
                scene_manager_set_scene_state(
                    nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoData);
                view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
                consumed = true;
            } else if(nfc->dev.format == NfcDeviceSaveFormatBankCard) {
                scene_manager_set_scene_state(
                    nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoData);
                view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewBankCard);
                consumed = true;
            }
        } else if(state == NfcSceneDeviceInfoData) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoUid);
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            consumed = true;
        }
    }
    return consumed;
}

const void nfc_scene_device_info_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear Custom Widget
    widget_clear(nfc->widget);

    if(nfc->dev.format == NfcDeviceSaveFormatUid) {
        // Clear Dialog
        DialogEx* dialog_ex = nfc->dialog_ex;
        dialog_ex_set_header(dialog_ex, NULL, 0, 0, AlignCenter, AlignCenter);
        dialog_ex_set_text(dialog_ex, NULL, 0, 0, AlignCenter, AlignTop);
        dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
        dialog_ex_set_left_button_text(dialog_ex, NULL);
        dialog_ex_set_right_button_text(dialog_ex, NULL);
        dialog_ex_set_center_button_text(dialog_ex, NULL);
        dialog_ex_set_result_callback(dialog_ex, NULL);
        dialog_ex_set_context(dialog_ex, NULL);
    } else if(nfc->dev.format == NfcDeviceSaveFormatMifareUl) {
        // Clear TextBox
        text_box_clean(nfc->text_box);
        string_clean(nfc->text_box_store);
    } else if(nfc->dev.format == NfcDeviceSaveFormatBankCard) {
        // Clear Bank Card
        bank_card_clear(nfc->bank_card);
    }
}
