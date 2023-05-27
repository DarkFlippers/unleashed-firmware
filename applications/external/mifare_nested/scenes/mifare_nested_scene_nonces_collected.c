#include "../mifare_nested_i.h"

void mifare_nested_scene_nonces_collected_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    MifareNested* mifare_nested = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(mifare_nested->view_dispatcher, result);
    }
}

void mifare_nested_scene_nonces_collected_on_enter(void* context) {
    MifareNested* mifare_nested = context;
    Widget* widget = mifare_nested->widget;

    notification_message(mifare_nested->notifications, &sequence_success);

    widget_add_icon_element(widget, 52, 17, &I_DolphinSuccess);
    widget_add_string_element(widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Nonces collected");
    widget_add_string_element(
        widget, 0, 12, AlignLeft, AlignTop, FontSecondary, "Now you can run");
    widget_add_string_element(widget, 0, 22, AlignLeft, AlignTop, FontSecondary, "script on your");
    widget_add_string_element(widget, 0, 32, AlignLeft, AlignTop, FontSecondary, "PC to recover");
    widget_add_string_element(widget, 0, 42, AlignLeft, AlignTop, FontSecondary, "keys");
    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Back",
        mifare_nested_scene_nonces_collected_widget_callback,
        mifare_nested);

    // Setup and start worker
    view_dispatcher_switch_to_view(mifare_nested->view_dispatcher, MifareNestedViewWidget);
}

bool mifare_nested_scene_nonces_collected_on_event(void* context, SceneManagerEvent event) {
    MifareNested* mifare_nested = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeCenter || event.event == GuiButtonTypeLeft) {
            scene_manager_search_and_switch_to_previous_scene(mifare_nested->scene_manager, 0);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_search_and_switch_to_previous_scene(mifare_nested->scene_manager, 0);
        consumed = true;
    }

    return consumed;
}

void mifare_nested_scene_nonces_collected_on_exit(void* context) {
    MifareNested* mifare_nested = context;

    widget_reset(mifare_nested->widget);
}
