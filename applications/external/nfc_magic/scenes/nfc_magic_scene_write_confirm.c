#include "../nfc_magic_i.h"

void nfc_magic_scene_write_confirm_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcMagic* nfc_magic = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc_magic->view_dispatcher, result);
    }
}

void nfc_magic_scene_write_confirm_on_enter(void* context) {
    NfcMagic* nfc_magic = context;
    Widget* widget = nfc_magic->widget;

    widget_add_string_element(widget, 3, 0, AlignLeft, AlignTop, FontPrimary, "Risky operation");
    widget_add_text_box_element(
        widget,
        0,
        13,
        128,
        54,
        AlignLeft,
        AlignTop,
        "Writing to this card will change manufacturer block. On some cards it may not be rewritten",
        false);
    widget_add_button_element(
        widget,
        GuiButtonTypeCenter,
        "Continue",
        nfc_magic_scene_write_confirm_widget_callback,
        nfc_magic);
    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Back",
        nfc_magic_scene_write_confirm_widget_callback,
        nfc_magic);

    // Setup and start worker
    view_dispatcher_switch_to_view(nfc_magic->view_dispatcher, NfcMagicViewWidget);
}

bool nfc_magic_scene_write_confirm_on_event(void* context, SceneManagerEvent event) {
    NfcMagic* nfc_magic = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(nfc_magic->scene_manager);
        } else if(event.event == GuiButtonTypeCenter) {
            scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneWrite);
            consumed = true;
        }
    }
    return consumed;
}

void nfc_magic_scene_write_confirm_on_exit(void* context) {
    NfcMagic* nfc_magic = context;

    widget_reset(nfc_magic->widget);
}
