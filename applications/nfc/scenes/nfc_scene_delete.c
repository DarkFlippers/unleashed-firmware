#include "../nfc_i.h"

void nfc_scene_delete_widget_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_delete_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup Custom Widget view
    char temp_str[64];
    snprintf(temp_str, sizeof(temp_str), "\e#Delete %s?\e#", nfc->dev->dev_name);
    widget_add_text_box_element(
        nfc->widget, 0, 0, 128, 23, AlignCenter, AlignCenter, temp_str, false);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeLeft, "Back", nfc_scene_delete_widget_callback, nfc);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeRight, "Delete", nfc_scene_delete_widget_callback, nfc);
    FuriHalNfcDevData* data = &nfc->dev->dev_data.nfc_data;
    if(data->uid_len == 4) {
        snprintf(
            temp_str,
            sizeof(temp_str),
            "UID: %02X %02X %02X %02X",
            data->uid[0],
            data->uid[1],
            data->uid[2],
            data->uid[3]);
    } else if(data->uid_len == 7) {
        snprintf(
            temp_str,
            sizeof(temp_str),
            "UID: %02X %02X %02X %02X %02X %02X %02X",
            data->uid[0],
            data->uid[1],
            data->uid[2],
            data->uid[3],
            data->uid[4],
            data->uid[5],
            data->uid[6]);
    }
    widget_add_string_element(nfc->widget, 64, 23, AlignCenter, AlignTop, FontSecondary, temp_str);

    const char* protocol_name = NULL;
    NfcProtocol protocol = nfc->dev->dev_data.protocol;
    if(protocol == NfcDeviceProtocolEMV) {
        protocol_name = nfc_guess_protocol(protocol);
    } else if(protocol == NfcDeviceProtocolMifareUl) {
        protocol_name = nfc_mf_ul_type(nfc->dev->dev_data.mf_ul_data.type, false);
    }
    if(protocol_name) {
        widget_add_string_element(
            nfc->widget, 10, 33, AlignLeft, AlignTop, FontSecondary, protocol_name);
    }
    // TODO change dinamically
    widget_add_string_element(nfc->widget, 118, 33, AlignRight, AlignTop, FontSecondary, "NFC-A");
    snprintf(temp_str, sizeof(temp_str), "SAK: %02X", data->sak);
    widget_add_string_element(nfc->widget, 10, 43, AlignLeft, AlignTop, FontSecondary, temp_str);
    snprintf(temp_str, sizeof(temp_str), "ATQA: %02X%02X", data->atqa[0], data->atqa[1]);
    widget_add_string_element(nfc->widget, 118, 43, AlignRight, AlignTop, FontSecondary, temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_delete_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            return scene_manager_previous_scene(nfc->scene_manager);
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
