#include "../nfc_i.h"
#include "../helpers/nfc_emv_parser.h"

void nfc_scene_device_info_widget_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_device_info_on_enter(void* context) {
    Nfc* nfc = context;
    NfcDeviceData* dev_data = &nfc->dev->dev_data;

    FuriString* temp_str;
    temp_str = furi_string_alloc();

    if(dev_data->protocol == NfcDeviceProtocolEMV) {
        EmvData* emv_data = &dev_data->emv_data;
        furi_string_printf(temp_str, "\e#%s\n", emv_data->name);
        for(uint8_t i = 0; i < emv_data->number_len; i += 2) {
            furi_string_cat_printf(
                temp_str, "%02X%02X ", emv_data->number[i], emv_data->number[i + 1]);
        }
        furi_string_trim(temp_str);

        // Add expiration date
        if(emv_data->exp_mon) {
            furi_string_cat_printf(
                temp_str, "\nExp: %02X/%02X", emv_data->exp_mon, emv_data->exp_year);
        }
        // Parse currency code
        if((emv_data->currency_code)) {
            FuriString* currency_name;
            currency_name = furi_string_alloc();
            if(nfc_emv_parser_get_currency_name(
                   nfc->dev->storage, emv_data->currency_code, currency_name)) {
                furi_string_cat_printf(
                    temp_str, "\nCur: %s  ", furi_string_get_cstr(currency_name));
            }
            furi_string_free(currency_name);
        }
        // Parse country code
        if((emv_data->country_code)) {
            FuriString* country_name;
            country_name = furi_string_alloc();
            if(nfc_emv_parser_get_country_name(
                   nfc->dev->storage, emv_data->country_code, country_name)) {
                furi_string_cat_printf(temp_str, "Reg: %s", furi_string_get_cstr(country_name));
            }
            furi_string_free(country_name);
        }
    } else if(
        dev_data->protocol == NfcDeviceProtocolMifareClassic ||
        dev_data->protocol == NfcDeviceProtocolMifareDesfire ||
        dev_data->protocol == NfcDeviceProtocolMifareUl) {
        furi_string_set(temp_str, nfc->dev->dev_data.parsed_data);
    }

    widget_add_text_scroll_element(nfc->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    widget_add_button_element(
        nfc->widget, GuiButtonTypeRight, "More", nfc_scene_device_info_widget_callback, nfc);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_device_info_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneNfcDataInfo);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_device_info_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear views
    widget_reset(nfc->widget);
}
