#include "../nfc_i.h"

void nfc_scene_nfcv_read_success_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    furi_assert(context);
    Nfc* nfc = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_nfcv_read_success_on_enter(void* context) {
    Nfc* nfc = context;
    NfcDeviceData* dev_data = &nfc->dev->dev_data;
    FuriHalNfcDevData* nfc_data = &nfc->dev->dev_data.nfc_data;
    // Setup view
    Widget* widget = nfc->widget;
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", nfc_scene_nfcv_read_success_widget_callback, nfc);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "More", nfc_scene_nfcv_read_success_widget_callback, nfc);

    FuriString* temp_str = furi_string_alloc();

    switch(dev_data->nfcv_data.sub_type) {
    case NfcVTypePlain:
        furi_string_cat_printf(temp_str, "\e#ISO15693\n");
        break;
    case NfcVTypeSlix:
        furi_string_cat_printf(temp_str, "\e#ISO15693 SLIX\n");
        break;
    case NfcVTypeSlixS:
        furi_string_cat_printf(temp_str, "\e#ISO15693 SLIX-S\n");
        break;
    case NfcVTypeSlixL:
        furi_string_cat_printf(temp_str, "\e#ISO15693 SLIX-L\n");
        break;
    case NfcVTypeSlix2:
        furi_string_cat_printf(temp_str, "\e#ISO15693 SLIX2\n");
        break;
    default:
        furi_string_cat_printf(temp_str, "\e#ISO15693 (unknown)\n");
        break;
    }
    furi_string_cat_printf(temp_str, "UID:\n");
    for(size_t i = 0; i < nfc_data->uid_len; i++) {
        furi_string_cat_printf(temp_str, " %02X", nfc_data->uid[i]);
    }
    furi_string_cat_printf(temp_str, "\n");
    furi_string_cat_printf(temp_str, "(see More->Info for details)\n");

    widget_add_text_scroll_element(widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    notification_message_block(nfc->notifications, &sequence_set_green_255);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_nfcv_read_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRetryConfirm);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            // Clear device name
            nfc_device_set_name(nfc->dev, "");
            scene_manager_next_scene(nfc->scene_manager, NfcSceneNfcVMenu);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }

    return consumed;
}

void nfc_scene_nfcv_read_success_on_exit(void* context) {
    Nfc* nfc = context;

    notification_message_block(nfc->notifications, &sequence_reset_green);

    // Clear view
    widget_reset(nfc->widget);
}
