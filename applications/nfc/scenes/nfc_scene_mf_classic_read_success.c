#include "../nfc_i.h"
#include <dolphin/dolphin.h>

void nfc_scene_mf_classic_read_success_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    furi_assert(context);
    Nfc* nfc = context;

    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_mf_classic_read_success_on_enter(void* context) {
    Nfc* nfc = context;
    NfcDeviceData* dev_data = &nfc->dev->dev_data;
    MfClassicData* mf_data = &dev_data->mf_classic_data;

    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup view
    Widget* widget = nfc->widget;
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", nfc_scene_mf_classic_read_success_widget_callback, nfc);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "More", nfc_scene_mf_classic_read_success_widget_callback, nfc);

    string_t temp_str;
    if(string_size(nfc->dev->dev_data.parsed_data)) {
        string_init_set(temp_str, nfc->dev->dev_data.parsed_data);
    } else {
        string_init_printf(temp_str, "\e#%s\n", nfc_mf_classic_type(mf_data->type));
        string_cat_printf(temp_str, "UID:");
        for(size_t i = 0; i < dev_data->nfc_data.uid_len; i++) {
            string_cat_printf(temp_str, " %02X", dev_data->nfc_data.uid[i]);
        }
        uint8_t sectors_total = mf_classic_get_total_sectors_num(mf_data->type);
        uint8_t keys_total = sectors_total * 2;
        uint8_t keys_found = 0;
        uint8_t sectors_read = 0;
        mf_classic_get_read_sectors_and_keys(mf_data, &sectors_read, &keys_found);
        string_cat_printf(temp_str, "\nKeys Found: %d/%d", keys_found, keys_total);
        string_cat_printf(temp_str, "\nSectors Read: %d/%d", sectors_read, sectors_total);
    }

    widget_add_text_scroll_element(widget, 0, 0, 128, 52, string_get_cstr(temp_str));
    string_clear(temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_mf_classic_read_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRetryConfirm);
            consumed = true;
        } else if(event.event == GuiButtonTypeRight) {
            // Clear device name
            nfc_device_set_name(nfc->dev, "");
            scene_manager_next_scene(nfc->scene_manager, NfcSceneMfClassicMenu);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_next_scene(nfc->scene_manager, NfcSceneExitConfirm);
        consumed = true;
    }

    return consumed;
}

void nfc_scene_mf_classic_read_success_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear view
    widget_reset(nfc->widget);
}
