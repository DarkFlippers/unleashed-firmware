#include "../nfc_i.h"

#define NFC_SCENE_EMULATE_NFCV_LOG_SIZE_MAX (100)

enum {
    NfcSceneEmulateNfcVStateWidget,
    NfcSceneEmulateNfcVStateTextBox,
};

bool nfc_emulate_nfcv_worker_callback(NfcWorkerEvent event, void* context) {
    UNUSED(event);
    furi_assert(context);
    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventWorkerExit);
    return true;
}

void nfc_scene_emulate_nfcv_widget_callback(GuiButtonType result, InputType type, void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_emulate_nfcv_textbox_callback(void* context) {
    furi_assert(context);
    Nfc* nfc = context;
    view_dispatcher_send_custom_event(nfc->view_dispatcher, NfcCustomEventViewExit);
}

// Add widget with device name or inform that data received
static void nfc_scene_emulate_nfcv_widget_config(Nfc* nfc, bool data_received) {
    FuriHalNfcDevData* data = &nfc->dev->dev_data.nfc_data;
    Widget* widget = nfc->widget;
    widget_reset(widget);
    FuriString* info_str;
    info_str = furi_string_alloc();

    widget_add_icon_element(widget, 0, 3, &I_RFIDDolphinSend_97x61);
    widget_add_string_element(
        widget, 89, 32, AlignCenter, AlignTop, FontPrimary, "Emulating NfcV");
    if(strcmp(nfc->dev->dev_name, "")) {
        furi_string_printf(info_str, "%s", nfc->dev->dev_name);
    } else {
        for(uint8_t i = 0; i < data->uid_len; i++) {
            furi_string_cat_printf(info_str, "%02X ", data->uid[i]);
        }
    }
    furi_string_trim(info_str);
    widget_add_text_box_element(
        widget, 56, 43, 70, 21, AlignCenter, AlignTop, furi_string_get_cstr(info_str), true);
    furi_string_free(info_str);
    if(data_received) {
        widget_add_button_element(
            widget, GuiButtonTypeCenter, "Log", nfc_scene_emulate_nfcv_widget_callback, nfc);
    }
}

void nfc_scene_emulate_nfcv_on_enter(void* context) {
    Nfc* nfc = context;

    // Setup Widget
    nfc_scene_emulate_nfcv_widget_config(nfc, false);
    // Setup TextBox
    TextBox* text_box = nfc->text_box;
    text_box_set_font(text_box, TextBoxFontHex);
    text_box_set_focus(text_box, TextBoxFocusEnd);
    furi_string_reset(nfc->text_box_store);

    // Set Widget state and view
    scene_manager_set_scene_state(
        nfc->scene_manager, NfcSceneEmulateNfcV, NfcSceneEmulateNfcVStateWidget);
    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
    // Start worker
    memset(&nfc->dev->dev_data.reader_data, 0, sizeof(NfcReaderRequestData));
    nfc_worker_start(
        nfc->worker,
        NfcWorkerStateNfcVEmulate,
        &nfc->dev->dev_data,
        nfc_emulate_nfcv_worker_callback,
        nfc);

    nfc_blink_emulate_start(nfc);
}

bool nfc_scene_emulate_nfcv_on_event(void* context, SceneManagerEvent event) {
    Nfc* nfc = context;
    NfcVData* nfcv_data = &nfc->dev->dev_data.nfcv_data;
    uint32_t state = scene_manager_get_scene_state(nfc->scene_manager, NfcSceneEmulateNfcV);
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcCustomEventWorkerExit) {
            // Add data button to widget if data is received for the first time
            if(!furi_string_size(nfc->text_box_store)) {
                nfc_scene_emulate_nfcv_widget_config(nfc, true);
            }
            if(strlen(nfcv_data->last_command) > 0) {
                /* use the last n bytes from the log so there's enough space for the new log entry */
                size_t maxSize =
                    NFC_SCENE_EMULATE_NFCV_LOG_SIZE_MAX - (strlen(nfcv_data->last_command) + 1);
                if(furi_string_size(nfc->text_box_store) >= maxSize) {
                    furi_string_right(nfc->text_box_store, (strlen(nfcv_data->last_command) + 1));
                }
                furi_string_cat_printf(nfc->text_box_store, "%s", nfcv_data->last_command);
                furi_string_push_back(nfc->text_box_store, '\n');
                text_box_set_text(nfc->text_box, furi_string_get_cstr(nfc->text_box_store));

                /* clear previously logged command */
                strcpy(nfcv_data->last_command, "");
            }
            consumed = true;
        } else if(event.event == GuiButtonTypeCenter && state == NfcSceneEmulateNfcVStateWidget) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewTextBox);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneEmulateNfcV, NfcSceneEmulateNfcVStateTextBox);
            consumed = true;
        } else if(event.event == NfcCustomEventViewExit && state == NfcSceneEmulateNfcVStateTextBox) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneEmulateNfcV, NfcSceneEmulateNfcVStateWidget);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        if(state == NfcSceneEmulateNfcVStateTextBox) {
            view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
            scene_manager_set_scene_state(
                nfc->scene_manager, NfcSceneEmulateNfcV, NfcSceneEmulateNfcVStateWidget);
            consumed = true;
        }
    }

    return consumed;
}

void nfc_scene_emulate_nfcv_on_exit(void* context) {
    Nfc* nfc = context;

    // Stop worker
    nfc_worker_stop(nfc->worker);

    // Clear view
    widget_reset(nfc->widget);
    text_box_reset(nfc->text_box);
    furi_string_reset(nfc->text_box_store);

    nfc_blink_stop(nfc);
}
