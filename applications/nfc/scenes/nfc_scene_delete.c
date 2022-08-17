#include "../nfc_i.h"

void nfc_scene_delete_widget_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_delete_on_enter(void* context) {
    Nfc* nfc = context;
    FuriHalNfcDevData* nfc_data = &nfc->dev->dev_data.nfc_data;

    // Setup Custom Widget view
    string_t temp_str;
    string_init(temp_str);

    string_printf(temp_str, "\e#Delete %s?\e#", nfc->dev->dev_name);
    widget_add_text_box_element(
        nfc->widget, 0, 0, 128, 23, AlignCenter, AlignCenter, string_get_cstr(temp_str), false);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeLeft, "Cancel", nfc_scene_delete_widget_callback, nfc);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeRight, "Delete", nfc_scene_delete_widget_callback, nfc);

    string_set_str(temp_str, "UID:");
    for(size_t i = 0; i < nfc_data->uid_len; i++) {
        string_cat_printf(temp_str, " %02X", nfc_data->uid[i]);
    }
    widget_add_string_element(
        nfc->widget, 64, 24, AlignCenter, AlignTop, FontSecondary, string_get_cstr(temp_str));

    NfcProtocol protocol = nfc->dev->dev_data.protocol;
    if(protocol == NfcDeviceProtocolEMV) {
        string_set_str(temp_str, "EMV bank card");
    } else if(protocol == NfcDeviceProtocolMifareUl) {
        string_set_str(temp_str, nfc_mf_ul_type(nfc->dev->dev_data.mf_ul_data.type, true));
    } else if(protocol == NfcDeviceProtocolMifareClassic) {
        string_set_str(temp_str, nfc_mf_classic_type(nfc->dev->dev_data.mf_classic_data.type));
    } else if(protocol == NfcDeviceProtocolMifareDesfire) {
        string_set_str(temp_str, "MIFARE DESFire");
    } else {
        string_set_str(temp_str, "Unknown ISO tag");
    }
    widget_add_string_element(
        nfc->widget, 64, 34, AlignCenter, AlignTop, FontSecondary, string_get_cstr(temp_str));
    widget_add_string_element(nfc->widget, 64, 44, AlignCenter, AlignTop, FontSecondary, "NFC-A");
    string_clear(temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_delete_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            if(nfc_device_delete(nfc->dev, true)) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneDeleteSuccess);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneStart);
            }
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_delete_on_exit(void* context) {
    Nfc* nfc = context;

    widget_reset(nfc->widget);
}
