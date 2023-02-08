#include "gpio_app_i.h"

#include <furi.h>
#include <furi_hal.h>

static bool gpio_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    GpioApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool gpio_app_back_event_callback(void* context) {
    furi_assert(context);
    GpioApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void gpio_app_tick_event_callback(void* context) {
    furi_assert(context);
    GpioApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

GpioApp* gpio_app_alloc() {
    GpioApp* app = malloc(sizeof(GpioApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->gpio_items = gpio_items_alloc();

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&gpio_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, gpio_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, gpio_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, gpio_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        GpioAppViewVarItemList,
        variable_item_list_get_view(app->var_item_list));
    app->gpio_test = gpio_test_alloc(app->gpio_items);
    view_dispatcher_add_view(
        app->view_dispatcher, GpioAppViewGpioTest, gpio_test_get_view(app->gpio_test));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, GpioAppViewUsbUartCloseRpc, widget_get_view(app->widget));

    app->gpio_usb_uart = gpio_usb_uart_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, GpioAppViewUsbUart, gpio_usb_uart_get_view(app->gpio_usb_uart));

    view_dispatcher_add_view(
        app->view_dispatcher,
        GpioAppViewUsbUartCfg,
        variable_item_list_get_view(app->var_item_list));

    scene_manager_next_scene(app->scene_manager, GpioSceneStart);

    return app;
}

void gpio_app_free(GpioApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, GpioAppViewVarItemList);
    view_dispatcher_remove_view(app->view_dispatcher, GpioAppViewGpioTest);
    view_dispatcher_remove_view(app->view_dispatcher, GpioAppViewUsbUart);
    view_dispatcher_remove_view(app->view_dispatcher, GpioAppViewUsbUartCfg);
    view_dispatcher_remove_view(app->view_dispatcher, GpioAppViewUsbUartCloseRpc);
    variable_item_list_free(app->var_item_list);
    widget_free(app->widget);
    gpio_test_free(app->gpio_test);
    gpio_usb_uart_free(app->gpio_usb_uart);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Close records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);

    gpio_items_free(app->gpio_items);
    free(app);
}

int32_t gpio_app(void* p) {
    UNUSED(p);
    GpioApp* gpio_app = gpio_app_alloc();

    view_dispatcher_run(gpio_app->view_dispatcher);

    gpio_app_free(gpio_app);

    return 0;
}
