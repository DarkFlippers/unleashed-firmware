#include "../nfc_i.h"

void nfc_scene_delete_widget_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = (Nfc*)context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_delete_on_enter(void* context) {
    Nfc* nfc = (Nfc*)context;

    // Setup Custom Widget view
    char delete_str[64];
    snprintf(delete_str, sizeof(delete_str), "\e#Delete %s\e#", nfc->dev->dev_name);
    widget_add_text_box_element(nfc->widget, 0, 0, 128, 24, AlignCenter, AlignCenter, delete_str);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeLeft, "Back", nfc_scene_delete_widget_callback, nfc);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeRight, "Delete", nfc_scene_delete_widget_callback, nfc);
    char uid_str[32];
    NfcDeviceCommonData* data = &nfc->dev->dev_data.nfc_data;
    if(data->uid_len == 4) {
        snprintf(
            uid_str,
            sizeof(uid_str),
            "UID: %02X %02X %02X %02X",
            data->uid[0],
            data->uid[1],
            data->uid[2],
            data->uid[3]);
    } else if(data->uid_len == 7) {
        snprintf(
            uid_str,
            sizeof(uid_str),
            "UID: %02X %02X %02X %02X %02X %02X %02X",
            data->uid[0],
            data->uid[1],
            data->uid[2],
            data->uid[3],
            data->uid[4],
            data->uid[5],
            data->uid[6]);
    }
    widget_add_string_element(nfc->widget, 64, 21, AlignCenter, AlignTop, FontSecondary, uid_str);

    if(data->protocol > NfcDeviceProtocolUnknown) {
        widget_add_string_element(
            nfc->widget,
            10,
            32,
            AlignLeft,
            AlignTop,
            FontSecondary,
            nfc_get_protocol(data->protocol));
    }
    // TODO change dinamically
    widget_add_string_element(nfc->widget, 118, 32, AlignRight, AlignTop, FontSecondary, "NFC-A");
    char sak_str[16];
    snprintf(sak_str, sizeof(sak_str), "SAK: %02X", data->sak);
    widget_add_string_element(nfc->widget, 10, 42, AlignLeft, AlignTop, FontSecondary, sak_str);
    char atqa_str[16];
    snprintf(atqa_str, sizeof(atqa_str), "ATQA: %02X%02X", data->atqa[0], data->atqa[1]);
    widget_add_string_element(nfc->widget, 118, 42, AlignRight, AlignTop, FontSecondary, atqa_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_delete_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = (Nfc*)context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            return scene_manager_previous_scene(nfc->scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            if(nfc_device_delete(nfc->dev)) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneDeleteSuccess);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    nfc->scene_manager, NfcSceneStart);
            }
            return true;
        }
    }
    return false;
}

void nfc_scene_delete_on_exit(void* context) {
    Nfc* nfc = (Nfc*)context;

    widget_clear(nfc->widget);
}
