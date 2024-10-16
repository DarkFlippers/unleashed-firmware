#include "../bad_usb_app_i.h"

void bad_usb_scene_confirm_unpair_widget_callback(
    GuiButtonType type,
    InputType input_type,
    void* context) {
    UNUSED(input_type);
    SceneManagerEvent event = {.type = SceneManagerEventTypeCustom, .event = type};
    bad_usb_scene_confirm_unpair_on_event(context, event);
}

void bad_usb_scene_confirm_unpair_on_enter(void* context) {
    BadUsbApp* bad_usb = context;
    Widget* widget = bad_usb->widget;

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Cancel", bad_usb_scene_confirm_unpair_widget_callback, context);
    widget_add_button_element(
        widget,
        GuiButtonTypeRight,
        "Unpair",
        bad_usb_scene_confirm_unpair_widget_callback,
        context);

    widget_add_text_box_element(
        widget, 0, 0, 128, 64, AlignCenter, AlignTop, "\e#Unpair the Device?\e#\n", false);

    view_dispatcher_switch_to_view(bad_usb->view_dispatcher, BadUsbAppViewWidget);
}

bool bad_usb_scene_confirm_unpair_on_event(void* context, SceneManagerEvent event) {
    BadUsbApp* bad_usb = context;
    SceneManager* scene_manager = bad_usb->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(scene_manager, BadUsbSceneUnpairDone);
        } else if(event.event == GuiButtonTypeLeft) {
            scene_manager_previous_scene(scene_manager);
        }
    }

    return consumed;
}

void bad_usb_scene_confirm_unpair_on_exit(void* context) {
    BadUsbApp* bad_usb = context;
    Widget* widget = bad_usb->widget;

    widget_reset(widget);
}
