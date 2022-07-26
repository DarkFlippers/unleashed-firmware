#include "../nfc_i.h"
#include "../helpers/nfc_emv_parser.h"
#include <dolphin/dolphin.h>

void nfc_scene_emv_read_success_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_emv_read_success_on_enter(void* context) {
    Nfc* nfc = context;
    EmvData* emv_data = &nfc->dev->dev_data.emv_data;
    FuriHalNfcDevData* nfc_data = &nfc->dev->dev_data.nfc_data;
    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup Custom Widget view
    // Add frame
    widget_add_frame_element(nfc->widget, 0, 0, 128, 64, 6);
    // Add buttons
    widget_add_button_element(
        nfc->widget, GuiButtonTypeLeft, "Retry", nfc_scene_emv_read_success_widget_callback, nfc);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeRight, "Save", nfc_scene_emv_read_success_widget_callback, nfc);
    // Add card name
    widget_add_string_element(
        nfc->widget, 64, 3, AlignCenter, AlignTop, FontSecondary, nfc->dev->dev_data.emv_data.name);
    // Add card number
    string_t pan_str;
    string_init(pan_str);
    for(uint8_t i = 0; i < emv_data->number_len; i += 2) {
        string_cat_printf(pan_str, "%02X%02X ", emv_data->number[i], emv_data->number[i + 1]);
    }
    string_strim(pan_str);
    widget_add_string_element(
        nfc->widget, 64, 13, AlignCenter, AlignTop, FontSecondary, string_get_cstr(pan_str));
    string_clear(pan_str);
    // Parse country code
    string_t country_name;
    string_init(country_name);
    if((emv_data->country_code) &&
       nfc_emv_parser_get_country_name(nfc->dev->storage, emv_data->country_code, country_name)) {
        string_t disp_country;
        string_init_printf(disp_country, "Reg:%s", country_name);
        widget_add_string_element(
            nfc->widget, 7, 23, AlignLeft, AlignTop, FontSecondary, string_get_cstr(disp_country));
        string_clear(disp_country);
    }
    string_clear(country_name);
    // Parse currency code
    string_t currency_name;
    string_init(currency_name);
    if((emv_data->currency_code) &&
       nfc_emv_parser_get_currency_name(
           nfc->dev->storage, emv_data->currency_code, currency_name)) {
        string_t disp_currency;
        string_init_printf(disp_currency, "Cur:%s", currency_name);
        widget_add_string_element(
            nfc->widget,
            121,
            23,
            AlignRight,
            AlignTop,
            FontSecondary,
            string_get_cstr(disp_currency));
        string_clear(disp_currency);
    }
    string_clear(currency_name);
    char temp_str[32];
    // Add ATQA
    snprintf(temp_str, sizeof(temp_str), "ATQA: %02X%02X", nfc_data->atqa[0], nfc_data->atqa[1]);
    widget_add_string_element(nfc->widget, 121, 32, AlignRight, AlignTop, FontSecondary, temp_str);
    // Add UID
    snprintf(
        temp_str,
        sizeof(temp_str),
        "UID: %02X %02X %02X %02X",
        nfc_data->uid[0],
        nfc_data->uid[1],
        nfc_data->uid[2],
        nfc_data->uid[3]);
    widget_add_string_element(nfc->widget, 7, 42, AlignLeft, AlignTop, FontSecondary, temp_str);
    // Add SAK
    snprintf(temp_str, sizeof(temp_str), "SAK: %02X", nfc_data->sak);
    widget_add_string_element(nfc->widget, 121, 42, AlignRight, AlignTop, FontSecondary, temp_str);
    // Add expiration date
    if(emv_data->exp_mon) {
        char exp_str[16];
        snprintf(
            exp_str, sizeof(exp_str), "Exp: %02X/%02X", emv_data->exp_mon, emv_data->exp_year);
        widget_add_string_element(nfc->widget, 7, 32, AlignLeft, AlignTop, FontSecondary, exp_str);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_emv_read_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRetryConfirm);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            // Clear device name
            nfc_device_set_name(nfc->dev, "");
            nfc->dev->format = NfcDeviceSaveFormatBankCard;
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveName);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }
    return consumed;
}

void nfc_scene_emv_read_success_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    widget_reset(nfc->widget);
}
