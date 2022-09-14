#include "../nfc_i.h"
#include <dolphin/dolphin.h>

void nfc_scene_mf_ultralight_read_auth_result_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    Nfc* nfc = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_mf_ultralight_read_auth_result_on_enter(void* context) {
    Nfc* nfc = context;
    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup dialog view
    FuriHalNfcDevData* nfc_data = &nfc->dev->dev_data.nfc_data;
    MfUltralightData* mf_ul_data = &nfc->dev->dev_data.mf_ul_data;
    MfUltralightConfigPages* config_pages = mf_ultralight_get_config_pages(mf_ul_data);
    Widget* widget = nfc->widget;
    string_t temp_str;
    string_init(temp_str);

    if((mf_ul_data->data_read == mf_ul_data->data_size) && (mf_ul_data->data_read > 0)) {
        widget_add_string_element(
            widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "All pages are unlocked!");
    } else {
        widget_add_string_element(
            widget, 64, 0, AlignCenter, AlignTop, FontPrimary, "Not all pages unlocked!");
    }
    string_set_str(temp_str, "UID:");
    for(size_t i = 0; i < nfc_data->uid_len; i++) {
        string_cat_printf(temp_str, " %02X", nfc_data->uid[i]);
    }
    widget_add_string_element(
        widget, 0, 17, AlignLeft, AlignTop, FontSecondary, string_get_cstr(temp_str));
    if(mf_ul_data->auth_success) {
        string_printf(
            temp_str,
            "Password: %02X %02X %02X %02X",
            config_pages->auth_data.pwd.raw[0],
            config_pages->auth_data.pwd.raw[1],
            config_pages->auth_data.pwd.raw[2],
            config_pages->auth_data.pwd.raw[3]);
        widget_add_string_element(
            widget, 0, 28, AlignLeft, AlignTop, FontSecondary, string_get_cstr(temp_str));
        string_printf(
            temp_str,
            "PACK: %02X %02X",
            config_pages->auth_data.pack.raw[0],
            config_pages->auth_data.pack.raw[1]);
        widget_add_string_element(
            widget, 0, 39, AlignLeft, AlignTop, FontSecondary, string_get_cstr(temp_str));
    }
    string_printf(
        temp_str, "Pages Read: %d/%d", mf_ul_data->data_read / 4, mf_ul_data->data_size / 4);
    widget_add_string_element(
        widget, 0, 50, AlignLeft, AlignTop, FontSecondary, string_get_cstr(temp_str));
    widget_add_button_element(
        widget,
        GuiButtonTypeRight,
        "Save",
        nfc_scene_mf_ultralight_read_auth_result_widget_callback,
        nfc);

    string_clear(temp_str);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_ultralight_read_auth_result_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeRight) {
            nfc->dev->format = NfcDeviceSaveFormatMifareUl;
            // Clear device name
            nfc_device_set_name(nfc->dev, "");
            scene_manager_next_scene(nfc->scene_manager, NfcSceneSaveName);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            nfc->scene_manager, NfcSceneMfUltralightUnlockMenu);
    }

    return consumed;
}

void nfc_scene_mf_ultralight_read_auth_result_on_exit(void* context) {
    Nfc* nfc = context;

    // Clean views
    widget_reset(nfc->widget);
}
