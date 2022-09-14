#include "../nfc_i.h"

void nfc_scene_nfc_data_info_widget_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_nfc_data_info_on_enter(void* context) {
    Nfc* nfc = context;
    Widget* widget = nfc->widget;
    FuriHalNfcDevData* nfc_data = &nfc->dev->dev_data.nfc_data;
    NfcDeviceData* dev_data = &nfc->dev->dev_data;
    NfcProtocol protocol = dev_data->protocol;
    uint8_t text_scroll_height = 0;
    if((protocol == NfcDeviceProtocolMifareDesfire) || (protocol == NfcDeviceProtocolMifareUl)) {
        widget_add_button_element(
            widget, GuiButtonTypeRight, "More", nfc_scene_nfc_data_info_widget_callback, nfc);
        text_scroll_height = 52;
    } else {
        text_scroll_height = 64;
    }

    string_t temp_str;
    string_init(temp_str);
    // Set name if present
    if(nfc->dev->dev_name[0] != '\0') {
        string_printf(temp_str, "\ec%s\n", nfc->dev->dev_name);
    }

    // Set tag type
    if(protocol == NfcDeviceProtocolEMV) {
        string_cat_printf(temp_str, "\e#EMV Bank Card\n");
    } else if(protocol == NfcDeviceProtocolMifareUl) {
        string_cat_printf(temp_str, "\e#%s\n", nfc_mf_ul_type(dev_data->mf_ul_data.type, true));
    } else if(protocol == NfcDeviceProtocolMifareClassic) {
        string_cat_printf(
            temp_str, "\e#%s\n", nfc_mf_classic_type(dev_data->mf_classic_data.type));
    } else if(protocol == NfcDeviceProtocolMifareDesfire) {
        string_cat_printf(temp_str, "\e#MIFARE DESfire\n");
    } else {
        string_cat_printf(temp_str, "\e#Unknown ISO tag\n");
    }

    // Set tag iso data
    char iso_type = FURI_BIT(nfc_data->sak, 5) ? '4' : '3';
    string_cat_printf(temp_str, "ISO 14443-%c (NFC-A)\n", iso_type);
    string_cat_printf(temp_str, "UID:");
    for(size_t i = 0; i < nfc_data->uid_len; i++) {
        string_cat_printf(temp_str, " %02X", nfc_data->uid[i]);
    }
    string_cat_printf(temp_str, "\nATQA: %02X %02X ", nfc_data->atqa[1], nfc_data->atqa[0]);
    string_cat_printf(temp_str, " SAK: %02X", nfc_data->sak);

    // Set application specific data
    if(protocol == NfcDeviceProtocolMifareDesfire) {
        MifareDesfireData* data = &dev_data->mf_df_data;
        uint32_t bytes_total = 1 << (data->version.sw_storage >> 1);
        uint32_t bytes_free = data->free_memory ? data->free_memory->bytes : 0;
        string_cat_printf(temp_str, "\n%d", bytes_total);
        if(data->version.sw_storage & 1) {
            string_push_back(temp_str, '+');
        }
        string_cat_printf(temp_str, " bytes, %d bytes free\n", bytes_free);

        uint16_t n_apps = 0;
        uint16_t n_files = 0;
        for(MifareDesfireApplication* app = data->app_head; app; app = app->next) {
            n_apps++;
            for(MifareDesfireFile* file = app->file_head; file; file = file->next) {
                n_files++;
            }
        }
        string_cat_printf(temp_str, "%d Application", n_apps);
        if(n_apps != 1) {
            string_push_back(temp_str, 's');
        }
        string_cat_printf(temp_str, ", %d file", n_files);
        if(n_files != 1) {
            string_push_back(temp_str, 's');
        }
    } else if(protocol == NfcDeviceProtocolMifareUl) {
        MfUltralightData* data = &dev_data->mf_ul_data;
        string_cat_printf(
            temp_str, "\nPages Read %d/%d", data->data_read / 4, data->data_size / 4);
        if(data->data_size > data->data_read) {
            string_cat_printf(temp_str, "\nPassword-protected");
        }
    } else if(protocol == NfcDeviceProtocolMifareClassic) {
        MfClassicData* data = &dev_data->mf_classic_data;
        uint8_t sectors_total = mf_classic_get_total_sectors_num(data->type);
        uint8_t keys_total = sectors_total * 2;
        uint8_t keys_found = 0;
        uint8_t sectors_read = 0;
        mf_classic_get_read_sectors_and_keys(data, &sectors_read, &keys_found);
        string_cat_printf(temp_str, "\nKeys Found %d/%d", keys_found, keys_total);
        string_cat_printf(temp_str, "\nSectors Read %d/%d", sectors_read, sectors_total);
    }

    // Add text scroll widget
    widget_add_text_scroll_element(
        widget, 0, 0, 128, text_scroll_height, string_get_cstr(temp_str));
    string_clear(temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_nfc_data_info_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    NfcProtocol protocol = nfc->dev->dev_data.protocol;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            if(protocol == NfcDeviceProtocolMifareDesfire) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfDesfireApp);
                consumed = true;
            } else if(protocol == NfcDeviceProtocolMifareUl) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightData);
                consumed = true;
            }
        }
    }

    return consumed;
}

void nfc_scene_nfc_data_info_on_exit(void* context) {
    Nfc* nfc = context;

    widget_reset(nfc->widget);
}