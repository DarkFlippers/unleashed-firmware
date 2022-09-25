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

    string_t temp_str;
    string_init(temp_str);

    if(dev_data->protocol == NfcDeviceProtocolEMV) {
        EmvData* emv_data = &dev_data->emv_data;
        string_printf(temp_str, "\e#%s\n", emv_data->name);
        for(uint8_t i = 0; i < emv_data->number_len; i += 2) {
            string_cat_printf(temp_str, "%02X%02X ", emv_data->number[i], emv_data->number[i + 1]);
        }
        string_strim(temp_str);

        // Add expiration date
        if(emv_data->exp_mon) {
            string_cat_printf(temp_str, "\nExp: %02X/%02X", emv_data->exp_mon, emv_data->exp_year);
        }
        // Parse currency code
        if((emv_data->currency_code)) {
            string_t currency_name;
            string_init(currency_name);
            if(nfc_emv_parser_get_currency_name(
                   nfc->dev->storage, emv_data->currency_code, currency_name)) {
                string_cat_printf(temp_str, "\nCur: %s  ", string_get_cstr(currency_name));
            }
            string_clear(currency_name);
        }
        // Parse country code
        if((emv_data->country_code)) {
            string_t country_name;
            string_init(country_name);
            if(nfc_emv_parser_get_country_name(
                   nfc->dev->storage, emv_data->country_code, country_name)) {
                string_cat_printf(temp_str, "Reg: %s", string_get_cstr(country_name));
            }
            string_clear(country_name);
        }
    } else if(
        dev_data->protocol == NfcDeviceProtocolMifareClassic ||
        dev_data->protocol == NfcDeviceProtocolMifareUl) {
        string_set(temp_str, nfc->dev->dev_data.parsed_data);
    }

    widget_add_text_scroll_element(nfc->widget, 0, 0, 128, 52, string_get_cstr(temp_str));
    string_clear(temp_str);

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
