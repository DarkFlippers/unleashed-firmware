#include "../gpio_app_i.h"
#include "../gpio_custom_event.h"

void gpio_scene_usb_uart_close_rpc_on_enter(void* context) {
    GpioApp* app = context;

    widget_add_icon_element(app->widget, 78, 0, &I_ActiveConnection_50x64);
    widget_add_string_multiline_element(
        app->widget, 3, 2, AlignLeft, AlignTop, FontPrimary, "Connection\nIs Active!");
    widget_add_string_multiline_element(
        app->widget,
        3,
        30,
        AlignLeft,
        AlignTop,
        FontSecondary,
        "Disconnect from\nPC or phone to\nuse this function.");

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
