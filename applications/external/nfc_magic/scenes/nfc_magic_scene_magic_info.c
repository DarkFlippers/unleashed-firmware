#include "../nfc_magic_i.h"
#include "../lib/magic/types.h"

void nfc_magic_scene_magic_info_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcMagic* nfc_magic = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc_magic->view_dispatcher, result);
    }
}

void nfc_magic_scene_magic_info_on_enter(void* context) {
    NfcMagic* nfc_magic = context;
    Widget* widget = nfc_magic->widget;
    const char* card_type = nfc_magic_type(nfc_magic->dev->type);

    notification_message(nfc_magic->notifications, &sequence_success);

    widget_add_icon_element(widget, 73, 17, &I_DolphinCommon_56x48);
    widget_add_string_element(
        widget, 3, 4, AlignLeft, AlignTop, FontPrimary, "Magic card detected");
    widget_add_string_element(widget, 3, 17, AlignLeft, AlignTop, FontSecondary, card_type);
    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Retry", nfc_magic_scene_magic_info_widget_callback, nfc_magic);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "More", nfc_magic_scene_magic_info_widget_callback, nfc_magic);

    // Setup and start worker
    view_dispatcher_switch_to_view(nfc_magic->view_dispatcher, NfcMagicViewWidget);
}

bool nfc_magic_scene_magic_info_on_event(void* context, SceneManagerEvent event) {
    NfcMagic* nfc_magic = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(nfc_magic->scene_manager);
        } else if(event.event == GuiButtonTypeRight) {
            MagicType type = nfc_magic->dev->type;
            if(type == MagicTypeGen4) {
                scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneGen4Actions);
                consumed = true;
            } else {
                scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneActions);
                consumed = true;
            }
        }
    }
    return consumed;
}

void nfc_magic_scene_magic_info_on_exit(void* context) {
    NfcMagic* nfc_magic = context;

    widget_reset(nfc_magic->widget);
}
