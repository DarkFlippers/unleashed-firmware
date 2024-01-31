#include "../nfc_app_i.h"

void nfc_scene_delete_widget_callback(GuiButtonType result, InputType type, void* context) {
    NfcApp* nfc = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc->view_dispatcher, result);
    }
}

void nfc_scene_delete_on_enter(void* context) {
    NfcApp* nfc = context;

    // Setup Custom Widget view
    FuriString* temp_str;
    temp_str = furi_string_alloc();

    furi_string_printf(temp_str, "\e#Delete %s?\e#", furi_string_get_cstr(nfc->file_name));
    widget_add_text_box_element(
        nfc->widget, 0, 0, 128, 23, AlignCenter, AlignCenter, furi_string_get_cstr(temp_str), false);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeLeft, "Cancel", nfc_scene_delete_widget_callback, nfc);
    widget_add_button_element(
        nfc->widget, GuiButtonTypeRight, "Delete", nfc_scene_delete_widget_callback, nfc);

    size_t uid_len;
    const uint8_t* uid = nfc_device_get_uid(nfc->nfc_device, &uid_len);

    furi_string_set(temp_str, "UID:");
    for(size_t i = 0; i < uid_len; i++) {
        furi_string_cat_printf(temp_str, " %02X", uid[i]);
    }
    widget_add_string_element(
        nfc->widget, 64, 24, AlignCenter, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));

    furi_string_set_str(temp_str, nfc_device_get_name(nfc->nfc_device, NfcDeviceNameTypeFull));
    widget_add_string_element(
        nfc->widget, 64, 34, AlignCenter, AlignTop, FontSecondary, furi_string_get_cstr(temp_str));
    furi_string_free(temp_str);

    view_dispatcher_switch_to_view(nfc->view_dispatcher, NfcViewWidget);
}

bool nfc_scene_delete_on_event(void* context, SceneManagerEvent event) {
    NfcApp* nfc = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(nfc->scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            if(nfc_delete(nfc)) {
                scene_manager_next_scene(nfc->scene_manager, NfcSceneDeleteSuccess);
            } else {
                if(!scene_manager_search_and_switch_to_previous_scene(
                       nfc->scene_manager, NfcSceneStart)) {
                    scene_manager_stop(nfc->scene_manager);
                    view_dispatcher_stop(nfc->view_dispatcher);
                }
            }
            consumed = true;
        }
    }
    return consumed;
}

void nfc_scene_delete_on_exit(void* context) {
    NfcApp* nfc = context;

    widget_reset(nfc->widget);
}
