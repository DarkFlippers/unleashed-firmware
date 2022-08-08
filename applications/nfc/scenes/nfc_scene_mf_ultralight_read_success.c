#include "../nfc_i.h"
#include <dolphin/dolphin.h>

enum {
    ReadMifareUlStateShowInfo,
    ReadMifareUlStateShowData,
};

void nfc_scene_mf_ultralight_read_success_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    Nfc* nfc = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_mf_ultralight_read_success_on_enter(void* context) {
    Nfc* nfc = context;
    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup widget view
    FuriHalNfcDevData* data = &nfc->dev->dev_data.nfc_data;
    MfUltralightData* mf_ul_data = &nfc->dev->dev_data.mf_ul_data;
    Widget* widget = nfc->widget;
    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Retry",
        nfc_scene_mf_ultralight_read_success_widget_callback,
        nfc);
    widget_add_button_element(
        widget,
        GuiButtonTypeCenter,
        "Data",
        nfc_scene_mf_ultralight_read_success_widget_callback,
        nfc);
    widget_add_button_element(
        widget,
        GuiButtonTypeRight,
        "More",
        nfc_scene_mf_ultralight_read_success_widget_callback,
        nfc);

    widget_add_string_element(
        widget, 0, 0, AlignLeft, AlignTop, FontSecondary, nfc_mf_ul_type(mf_ul_data->type, true));
    string_t data_str;
    string_init_printf(data_str, "UID:");
    for(size_t i = 0; i < data->uid_len; i++) {
        string_cat_printf(data_str, " %02X", data->uid[i]);
    }
    widget_add_string_element(
        widget, 0, 13, AlignLeft, AlignTop, FontSecondary, string_get_cstr(data_str));
    string_printf(
        data_str, "Pages Read: %d/%d", mf_ul_data->data_read / 4, mf_ul_data->data_size / 4);
    widget_add_string_element(
        widget, 0, 24, AlignLeft, AlignTop, FontSecondary, string_get_cstr(data_str));
    if(mf_ul_data->data_read != mf_ul_data->data_size) {
        widget_add_string_element(
            widget, 0, 35, AlignLeft, AlignTop, FontSecondary, "Password-protected pages!");
    }
    string_clear(data_str);

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
        nfc->scene_manager, NfcSceneMfUltralightReadSuccess, ReadMifareUlStateShowInfo);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_ultralight_read_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;
    uint32_t state =
        scene_manager_get_scene_state(nfc->scene_manager, NfcSceneMfUltralightReadSuccess);

    if(event.type == SceneManagerEventTypeCustom) {
        if(state == ReadMifareUlStateShowInfo && event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRetryConfirm);
            consumed = true;
        } else if(state == ReadMifareUlStateShowInfo && event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfUltralightMenu);
            consumed = true;
        } else if(state == ReadMifareUlStateShowInfo && event.event == GuiButtonTypeCenter) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfUltralightReadSuccess, ReadMifareUlStateShowData);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state == ReadMifareUlStateShowData) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneMfUltralightReadSuccess, ReadMifareUlStateShowInfo);
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
    widget_reset(nfc->widget);
    text_box_reset(nfc->text_box);
    string_reset(nfc->text_box_store);
}
