#include "../gpio_app_i.h"
#include "../gpio_custom_event.h"

static void gpio_scene_usb_uart_close_rpc_event_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    furi_assert(context);
    GpioApp* app = context;

    if((result == GuiButtonTypeLeft) && (type == InputTypeShort)) {
        view_dispatcher_send_custom_event(app->view_dispatcher, GpioCustomEventErrorBack);
    }
}

void gpio_scene_usb_uart_close_rpc_on_enter(void* context) {
    GpioApp* app = context;

    widget_add_string_multiline_element(
        app->widget,
        63,
        10,
        AlignCenter,
        AlignTop,
        FontSecondary,
        "Disconnect from\ncompanion app\nto use this function");

    widget_add_button_element(
        app->widget, GuiButtonTypeLeft, "Back", gpio_scene_usb_uart_close_rpc_event_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, GpioAppViewUsbUartCloseRpc);
}

bool gpio_scene_usb_uart_close_rpc_on_event(void* context, SceneManagerEvent event) {
    GpioApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GpioCustomEventErrorBack) {
            if(!scene_manager_previous_scene(app->scene_manager)) {
                scene_manager_stop(app->scene_manager);
                view_dispatcher_stop(app->view_dispatcher);
            }
            consumed = true;
        }
    }
    return consumed;
}

void gpio_scene_usb_uart_close_rpc_on_exit(void* context) {
    GpioApp* app = context;
    widget_reset(app->widget);
}
