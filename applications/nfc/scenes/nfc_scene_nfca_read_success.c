#include "../nfc_i.h"
#include <dolphin/dolphin.h>

void nfc_scene_nfca_read_success_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    furi_assert(context);
    Nfc* nfc = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_nfca_read_success_on_enter(void* context) {
    Nfc* nfc = context;

    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup view
    FuriHalNfcDevData* data = &nfc->dev->dev_data.nfc_data;
    Widget* widget = nfc->widget;

    string_t temp_str;
    string_init_set_str(temp_str, "\e#Unknown ISO tag\n");

    char iso_type = FURI_BIT(data->sak, 5) ? '4' : '3';
    string_cat_printf(temp_str, "ISO 14443-%c (NFC-A)\n", iso_type);
    string_cat_printf(temp_str, "UID:");
    for(size_t i = 0; i < data->uid_len; i++) {
        string_cat_printf(temp_str, " %02X", data->uid[i]);
    }
    string_cat_printf(temp_str, "\nATQA: %02X %02X ", data->atqa[1], data->atqa[0]);
    string_cat_printf(temp_str, " SAK: %02X", data->sak);

    widget_add_text_scroll_element(widget, 0, 0, 128, 52, string_get_cstr(temp_str));
    string_clear(temp_str);

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", nfc_scene_nfca_read_success_widget_callback, nfc);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "More", nfc_scene_nfca_read_success_widget_callback, nfc);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_nfca_read_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRetryConfirm);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneNfcaMenu);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }
    return consumed;
}

void nfc_scene_nfca_read_success_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    widget_reset(nfc->widget);
}
