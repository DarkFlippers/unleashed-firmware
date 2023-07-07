#include "../mifare_nested_i.h"

void mifare_nested_scene_added_keys_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    MifareNested* mifare_nested = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(mifare_nested->view_dispatcher, result);
    }
}

void mifare_nested_scene_added_keys_on_enter(void* context) {
    MifareNested* mifare_nested = context;
    KeyInfo_t* key_info = mifare_nested->keys;
    Widget* widget = mifare_nested->widget;
    char draw_str[32] = {};
    char append[5] = {'k', 'e', 'y', ' ', '\0'};
    if(key_info->added_keys != 1) {
        append[3] = 's';
    }

    widget_add_string_element(
        widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "Results of key recovery");

    if(key_info->added_keys != 0) {
        snprintf(draw_str, sizeof(draw_str), "Added: %lu %s", key_info->added_keys, append);
        notification_message(mifare_nested->notifications, &sequence_success);
        widget_add_icon_element(widget, 52, 17, &I_DolphinSuccess);
    } else {
        snprintf(draw_str, sizeof(draw_str), "No new keys were added");
        widget_add_string_element(
            widget, 0, 22, AlignLeft, AlignTop, FontSecondary, "Try running \"Nested attack\"");
        widget_add_string_element(widget, 0, 32, AlignLeft, AlignTop, FontSecondary, "again");
        notification_message(mifare_nested->notifications, &sequence_error);
    }

    widget_add_string_element(widget, 0, 12, AlignLeft, AlignTop, FontSecondary, draw_str);
    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Back",
        mifare_nested_scene_added_keys_widget_callback,
        mifare_nested);

    free(key_info);

    KeyInfo_t* new_key_info = malloc(sizeof(KeyInfo_t));
    mifare_nested->keys = new_key_info;

    // Setup and start worker
    view_dispatcher_switch_to_view(mifare_nested->view_dispatcher, MifareNestedViewWidget);
}

bool mifare_nested_scene_added_keys_on_event(void* context, SceneManagerEvent event) {
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

void mifare_nested_scene_added_keys_on_exit(void* context) {
    MifareNested* mifare_nested = context;

    widget_reset(mifare_nested->widget);
}
