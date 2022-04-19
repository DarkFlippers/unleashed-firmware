#include "../nfc_i.h"
#include "../helpers/nfc_emv_parser.h"
#include <dolphin/dolphin.h>

void nfc_scene_read_emv_app_widget_callback(GuiButtonType result, InputType type, void* context) {
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_read_emv_app_success_on_enter(void* context) {
    Nfc* nfc = context;
    DOLPHIN_DEED(DolphinDeedNfcReadSuccess);

    // Setup view
    FuriHalNfcDevData* nfc_data = &nfc->dev->dev_data.nfc_data;
    EmvData* emv_data = &nfc->dev->dev_data.emv_data;
    Widget* widget = nfc->widget;
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", nfc_scene_read_emv_app_widget_callback, nfc);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "Run app", nfc_scene_read_emv_app_widget_callback, nfc);
    widget_add_string_element(widget, 36, 5, AlignLeft, AlignTop, FontPrimary, "Found EMV App");
    widget_add_icon_element(widget, 8, 5, &I_Medium_chip_22x21);
    // Display UID
    string_t temp_str;
    string_init_printf(temp_str, "UID:");
    for(size_t i = 0; i < nfc_data->uid_len; i++) {
        string_cat_printf(temp_str, " %02X", nfc_data->uid[i]);
    }
    widget_add_string_element(
        widget, 36, 18, AlignLeft, AlignTop, FontSecondary, string_get_cstr(temp_str));
    string_reset(temp_str);
    // Display application
    string_printf(temp_str, "App: ");
    string_t aid;
    string_init(aid);
    bool aid_found =
        nfc_emv_parser_get_aid_name(nfc->dev->storage, emv_data->aid, emv_data->aid_len, aid);
    if(!aid_found) {
        for(uint8_t i = 0; i < emv_data->aid_len; i++) {
            string_cat_printf(aid, "%02X", emv_data->aid[i]);
        }
    }
    string_cat(temp_str, aid);
    widget_add_string_element(
        widget, 7, 29, AlignLeft, AlignTop, FontSecondary, string_get_cstr(temp_str));
    string_clear(temp_str);
    string_clear(aid);

    // Send notification
    if(scene_manager_get_scene_state(nfc->scene_manager, NfcSceneReadEmvAppSuccess) ==
       NFC_SEND_NOTIFICATION_TRUE) {
        notification_message(nfc->notifications, &sequence_success);
        scene_manager_set_scene_state(
            nfc->scene_manager, NfcSceneReadEmvAppSuccess, NFC_SEND_NOTIFICATION_FALSE);
    }

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_read_emv_app_success_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(nfc->scene_manager, NfcSceneRunEmvAppConfirm);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_read_emv_app_success_on_exit(void* context) {
    Nfc* nfc = context;

    // Clear views
    widget_reset(nfc->widget);
}
