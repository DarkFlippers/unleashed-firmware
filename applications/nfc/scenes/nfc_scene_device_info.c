#include "../nfc_i.h"
#include "../helpers/nfc_emv_parser.h"

enum {
    NfcSceneDeviceInfoUid,
    NfcSceneDeviceInfoData,
};

void nfc_scene_device_info_widget_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_device_info_dialog_callback(DialogExResult result, void* context) {
    Nfc* nfc = context;
    if(result == DialogExResultLeft) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
    }
}

void nfc_scene_device_info_bank_card_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
    }
}

void nfc_scene_device_info_on_enter(void* context) {
    Nfc* nfc = context;

    bool data_display_supported = (nfc->dev->format == NfcDeviceSaveFormatUid) ||
                                  (nfc->dev->format == NfcDeviceSaveFormatMifareUl) ||
                                  (nfc->dev->format == NfcDeviceSaveFormatMifareDesfire) ||
                                  (nfc->dev->format == NfcDeviceSaveFormatBankCard);
    // Setup Custom Widget view
    widget_add_text_box_element(
        nfc->widget, 0, 0, 128, 22, AlignCenter, AlignTop, nfc->dev->dev_name);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeLeft, "Back", nfc_scene_device_info_widget_callback, nfc);
    if(data_display_supported) {
        widget_add_button_element(
            nfc->widget, GuiButtonTypeRight, "Data", nfc_scene_device_info_widget_callback, nfc);
    }
    char uid_str[32];
    NfcDeviceCommonData* data = &nfc->dev->dev_data.nfc_data;
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

    const char* protocol_name = NULL;
    if(data->protocol == NfcDeviceProtocolEMV ||
       data->protocol == NfcDeviceProtocolMifareDesfire) {
        protocol_name = nfc_guess_protocol(data->protocol);
    } else if(data->protocol == NfcDeviceProtocolMifareUl) {
        protocol_name = nfc_mf_ul_type(nfc->dev->dev_data.mf_ul_data.type, false);
    } else if(data->protocol == NfcDeviceProtocolMifareClassic) {
        protocol_name = nfc_mf_classic_type(nfc->dev->dev_data.mf_classic_data.type);
    }
    if(protocol_name) {
        widget_add_string_element(
            nfc->widget, 10, 32, AlignLeft, AlignTop, FontSecondary, protocol_name);
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
    if(nfc->dev->format == NfcDeviceSaveFormatUid) {
        DialogEx* dialog_ex = nfc->dialog_ex;
        dialog_ex_set_left_button_text(dialog_ex, "Back");
        dialog_ex_set_text(dialog_ex, "No data", 64, 32, AlignCenter, AlignCenter);
        dialog_ex_set_context(dialog_ex, nfc);
        dialog_ex_set_result_callback(dialog_ex, nfc_scene_device_info_dialog_callback);
    } else if(nfc->dev->format == NfcDeviceSaveFormatMifareUl) {
        MifareUlData* mf_ul_data = &nfc->dev->dev_data.mf_ul_data;
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
    } else if(nfc->dev->format == NfcDeviceSaveFormatMifareDesfire) {
        MifareDesfireData* mf_df_data = &nfc->dev->dev_data.mf_df_data;
        uint16_t n_apps = 0;
        uint16_t n_files = 0;
        for(MifareDesfireApplication* app = mf_df_data->app_head; app; app = app->next) {
            n_apps++;
            for(MifareDesfireFile* file = app->file_head; file; file = file->next) {
                n_files++;
            }
        }
        nfc_text_store_set(
            nfc,
            "%d application%s, %d file%s",
            n_apps,
            n_apps == 1 ? "" : "s",
            n_files,
            n_files == 1 ? "" : "s");
        widget_add_string_element(
            nfc->widget, 64, 17, AlignCenter, AlignBottom, FontSecondary, nfc->text_store);
    } else if(nfc->dev->format == NfcDeviceSaveFormatBankCard) {
        NfcEmvData* emv_data = &nfc->dev->dev_data.emv_data;
        BankCard* bank_card = nfc->bank_card;
        bank_card_set_name(bank_card, emv_data->name);
        bank_card_set_number(bank_card, emv_data->number, emv_data->number_len);
        bank_card_set_back_callback(bank_card, nfc_scene_device_info_bank_card_callback, nfc);
        if(emv_data->exp_mon) {
            bank_card_set_exp_date(bank_card, emv_data->exp_mon, emv_data->exp_year);
        }
        string_t display_str;
        string_init(display_str);
        if(emv_data->country_code) {
            string_t country_name;
            string_init(country_name);
            if(nfc_emv_parser_get_country_name(
                   nfc->dev->storage, emv_data->country_code, country_name)) {
                string_printf(display_str, "Reg:%s", string_get_cstr(country_name));
                bank_card_set_country_name(bank_card, string_get_cstr(display_str));
            }
            string_clear(country_name);
        }
        if(emv_data->currency_code) {
            string_t currency_name;
            string_init(currency_name);
            if(nfc_emv_parser_get_currency_name(
                   nfc->dev->storage, emv_data->country_code, currency_name)) {
                string_printf(display_str, "Cur:%s", string_get_cstr(currency_name));
                bank_card_set_currency_name(bank_card, string_get_cstr(display_str));
            }
            string_clear(currency_name);
        }
        string_clear(display_str);
    }
    scene_manager_set_scene_state(nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoUid);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_device_info_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;
    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneDeviceInfo);

    if(event.type == SceneManagerEventTypeCustom) {
        if((state == NfcSceneDeviceInfoUid) && (event.event == GuiButtonTypeLeft)) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else if((state == NfcSceneDeviceInfoUid) && (event.event == GuiButtonTypeRight)) {
            if(nfc->dev->format == NfcDeviceSaveFormatUid) {
                scene_manager_set_scene_state(
                    nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoData);
                view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewDialogEx);
                consumed = true;
            } else if(nfc->dev->format == NfcDeviceSaveFormatMifareUl) {
                scene_manager_set_scene_state(
                    nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoData);
                view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
                consumed = true;
            } else if(nfc->dev->format == NfcDeviceSaveFormatBankCard) {
                scene_manager_set_scene_state(
                    nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoData);
                view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewBankCard);
                consumed = true;
            } else if(nfc->dev->format == NfcDeviceSaveFormatMifareDesfire) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMifareDesfireData);
                consumed = true;
            }
        } else if(state == NfcSceneDeviceInfoData && event.event == NfcCustomEventViewExit) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoUid);
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state == NfcSceneDeviceInfoData) {
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneDeviceInfo, NfcSceneDeviceInfoUid);
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_device_info_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Clear Custom Widget
    widget_reset(nfc->widget);

    if(nfc->dev->format == NfcDeviceSaveFormatUid) {
        // Clear Dialog
        DialogEx* dialog_ex = nfc->dialog_ex;
        dialog_ex_reset(dialog_ex);
    } else if(nfc->dev->format == NfcDeviceSaveFormatMifareUl) {
        // Clear TextBox
        text_box_reset(nfc->text_box);
        string_reset(nfc->text_box_store);
    } else if(nfc->dev->format == NfcDeviceSaveFormatBankCard) {
        // Clear Bank Card
        bank_card_clear(nfc->bank_card);
    }
}
