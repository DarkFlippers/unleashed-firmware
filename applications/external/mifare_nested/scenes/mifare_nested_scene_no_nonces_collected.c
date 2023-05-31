#include "../mifare_nested_i.h"

void mifare_nested_scene_no_nonces_collected_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    MifareNested* mifare_nested = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(mifare_nested->view_dispatcher, result);
    }
}

void mifare_nested_scene_no_nonces_collected_on_enter(void* context) {
    MifareNested* mifare_nested = context;
    Widget* widget = mifare_nested->widget;
    SaveNoncesResult_t* save_state = mifare_nested->save_state;

    notification_message(mifare_nested->notifications, &sequence_error);

    widget_add_icon_element(widget, 73, 12, &I_DolphinCry);
    widget_add_string_element(
        widget, 0, 0, AlignLeft, AlignTop, FontPrimary, "No nonces collected");

    uint32_t index = 12;

    if(save_state->skipped) {
        char append_skipped[8] = {'s', 'e', 'c', 't', 'o', 'r', ' ', '\0'};
        if(save_state->skipped != 1) {
            append_skipped[6] = 's';
        }

        char draw_str[32] = {};
        snprintf(
            draw_str, sizeof(draw_str), "Skipped: %lu %s", save_state->skipped, append_skipped);

        widget_add_string_element(widget, 0, index, AlignLeft, AlignTop, FontSecondary, draw_str);

        widget_add_string_element(
            widget, 0, index + 10, AlignLeft, AlignTop, FontSecondary, "(already has keys)");

        index += 20;
    }

    if(save_state->invalid) {
        char append_invalid[8] = {'s', 'e', 'c', 't', 'o', 'r', ' ', '\0'};
        if(save_state->invalid != 1) {
            append_invalid[6] = 's';
        }

        char draw_str[32] = {};
        snprintf(
            draw_str, sizeof(draw_str), "Invalid: %lu %s", save_state->invalid, append_invalid);

        widget_add_string_element(widget, 0, index, AlignLeft, AlignTop, FontSecondary, draw_str);

        widget_add_string_element(
            widget, 0, index + 10, AlignLeft, AlignTop, FontSecondary, "(can't auth)");
    }

    free(save_state);

    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Back",
        mifare_nested_scene_no_nonces_collected_widget_callback,
        mifare_nested);

    // Setup and start worker
    view_dispatcher_switch_to_view(mifare_nested->view_dispatcher, MifareNestedViewWidget);
}

bool mifare_nested_scene_no_nonces_collected_on_event(void* context, SceneManagerEvent event) {
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

void mifare_nested_scene_no_nonces_collected_on_exit(void* context) {
    MifareNested* mifare_nested = context;

    widget_reset(mifare_nested->widget);
}
