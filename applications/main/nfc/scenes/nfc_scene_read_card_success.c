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

    string_t temp_str;
    string_init(temp_str);
    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup view
    FuriHalNfcDevData* data = &nfc->dev->dev_data.nfc_data;
    Widget* widget = nfc->widget;
    string_set_str(temp_str, nfc_get_dev_type(data->type));
    widget_add_string_element(
        widget, 64, 12, AlignCenter, AlignBottom, FontPrimary, string_get_cstr(temp_str));
    string_set_str(temp_str, "UID:");
    for(uint8_t i = 0; i < data->uid_len; i++) {
        string_cat_printf(temp_str, " %02X", data->uid[i]);
    }
    widget_add_string_element(
        widget, 64, 32, AlignCenter, AlignCenter, FontSecondary, string_get_cstr(temp_str));
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", nfc_scene_read_card_success_widget_callback, nfc);

    string_clear(temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_read_card_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        }
    }
    return consumed;
}

void nfc_scene_read_card_success_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    widget_reset(nfc->widget);
}
