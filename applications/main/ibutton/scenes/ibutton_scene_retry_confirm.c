#include "../ibutton_i.h"

static void ibutton_scene_retry_confirm_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    iButton* ibutton = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(ibutton->view_dispatcher, result);
    }
}

void ibutton_scene_retry_confirm_on_enter(void* context) {
    iButton* ibutton = context;
    Widget* widget = ibutton->widget;

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Exit", ibutton_scene_retry_confirm_widget_callback, ibutton);
    widget_add_button_element(
        widget, GuiButtonTypeRight, "Stay", ibutton_scene_retry_confirm_widget_callback, ibutton);
    widget_add_string_element(
        widget, 64, 19, AlignCenter, AlignBottom, FontPrimary, "Retry Reading?");
    widget_add_string_element(
        widget, 64, 29, AlignCenter, AlignBottom, FontSecondary, "All unsaved data will be lost!");

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewWidget);
}

bool ibutton_scene_retry_confirm_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    SceneManager* scene_manager = ibutton->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        consumed = true; // Ignore Back button presses
    } else if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeLeft) {
            scene_manager_search_and_switch_to_previous_scene(scene_manager, iButtonSceneRead);
        } else if(event.event == GuiButtonTypeRight) {
            scene_manager_previous_scene(scene_manager);
        }
    }

    return consumed;
}

void ibutton_scene_retry_confirm_on_exit(void* context) {
    iButton* ibutton = context;
    widget_reset(ibutton->widget);
}
