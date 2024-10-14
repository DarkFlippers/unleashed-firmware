#include "../bad_ble_app_i.h"

void bad_ble_scene_confirm_unpair_widget_callback(
    GuiButtonType type,
    InputType input_type,
    void* context) {
    UNUSED(input_type);
    SceneManagerEvent event = {.type = SceneManagerEventTypeCustom, .event = type};
    bad_ble_scene_confirm_unpair_on_event(context, event);
}

void bad_ble_scene_confirm_unpair_on_enter(void* context) {
    BadBleApp* bad_ble = context;
    Widget* widget = bad_ble->widget;

    widget_add_button_element(
        widget, GuiButtonTypeLeft, "Cancel", bad_ble_scene_confirm_unpair_widget_callback, context);
    widget_add_button_element(
        widget,
        GuiButtonTypeRight,
        "Unpair",
        bad_ble_scene_confirm_unpair_widget_callback,
        context);

    widget_add_text_box_element(
        widget, 0, 0, 128, 64, AlignCenter, AlignTop, "\e#Unpair the Device?\e#\n", false);

    view_dispatcher_switch_to_view(bad_ble->view_dispatcher, BadBleAppViewWidget);
}

bool bad_ble_scene_confirm_unpair_on_event(void* context, SceneManagerEvent event) {
    BadBleApp* bad_ble = context;
    SceneManager* scene_manager = bad_ble->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == GuiButtonTypeRight) {
            scene_manager_next_scene(scene_manager, BadBleSceneUnpairDone);
        } else if(event.event == GuiButtonTypeLeft) {
            scene_manager_previous_scene(scene_manager);
        }
    }

    return consumed;
}

void bad_ble_scene_confirm_unpair_on_exit(void* context) {
    BadBleApp* bad_ble = context;
    Widget* widget = bad_ble->widget;

    widget_reset(widget);
}
