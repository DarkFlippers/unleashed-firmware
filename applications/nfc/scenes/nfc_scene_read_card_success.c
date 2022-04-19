#include "../nfc_i.h"
#include <dolphin/dolphin.h>

void nfc_scene_read_card_success_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    furi_assert(context);
    Nfc* nfc = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_read_card_success_on_enter(void* context) {
    Nfc* nfc = context;

    string_t data_str;
    string_t uid_str;
    string_init(data_str);
    string_init(uid_str);
    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Send notification
    notification_message(nfc->notifications, &sequence_success);

    // Setup view
    FuriHalNfcDevData* data = &nfc->dev->dev_data.nfc_data;
    Widget* widget = nfc->widget;
    string_set_str(data_str, nfc_get_dev_type(data->type));
    string_set_str(uid_str, "UID:");
    for(uint8_t i = 0; i < data->uid_len; i++) {
        string_cat_printf(uid_str, " %02X", data->uid[i]);
    }

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", nfc_scene_read_card_success_widget_callback, nfc);
    if(data->type == FuriHalNfcTypeA) {
        widget_add_button_element(
            widget, GuiButtonTypeRight, "More", nfc_scene_read_card_success_widget_callback, nfc);
        widget_add_icon_element(widget, 8, 13, &I_Medium_chip_22x21);
        string_cat_printf(data_str, " may be:");
        widget_add_string_element(
            widget, 37, 12, AlignLeft, AlignBottom, FontPrimary, string_get_cstr(data_str));
        string_printf(
            data_str,
            "%s\nATQA: %02X%02X SAK: %02X",
            nfc_guess_protocol(nfc->dev->dev_data.protocol),
            data->atqa[0],
            data->atqa[1],
            data->sak);
        widget_add_string_multiline_element(
            widget, 37, 16, AlignLeft, AlignTop, FontSecondary, string_get_cstr(data_str));
        widget_add_string_element(
            widget, 64, 46, AlignCenter, AlignBottom, FontSecondary, string_get_cstr(uid_str));
    } else {
        widget_add_string_element(
            widget, 64, 12, AlignCenter, AlignBottom, FontPrimary, string_get_cstr(data_str));
        widget_add_string_element(
            widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, string_get_cstr(uid_str));
    }

    string_clear(data_str);
    string_clear(uid_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_read_card_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    FuriHalNfcDevData* data = &nfc->dev->dev_data.nfc_data;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else if(data->type == FuriHalNfcTypeA && event.event == GuiButtonTypeRight) {
            // Clear device name
            nfc_device_set_name(nfc->dev, "");
            scene_manager_next_scene(nfc->scene_manager, NfcSceneCardMenu);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_read_card_success_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    widget_reset(nfc->widget);
}
