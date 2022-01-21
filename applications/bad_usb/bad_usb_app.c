#include "bad_usb_app_i.h"
#include <furi.h>
#include <furi_hal.h>

static bool bad_usb_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    BadUsbApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool bad_usb_app_back_event_callback(void* context) {
    furi_assert(context);
    BadUsbApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void bad_usb_app_tick_event_callback(void* context) {
    furi_assert(context);
    BadUsbApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

BadUsbApp* bad_usb_app_alloc() {
    BadUsbApp* app = furi_alloc(sizeof(BadUsbApp));

    app->gui = furi_record_open("gui");
    app->notifications = furi_record_open("notification");
    app->dialogs = furi_record_open("dialogs");

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&bad_usb_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, bad_usb_app_tick_event_callback, 500);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, bad_usb_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, bad_usb_app_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->bad_usb_view = bad_usb_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BadUsbAppViewWork, bad_usb_get_view(app->bad_usb_view));

    scene_manager_next_scene(app->scene_manager, BadUsbAppViewFileSelect);

    return app;
}

void bad_usb_app_free(BadUsbApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewFileSelect);
    view_dispatcher_remove_view(app->view_dispatcher, BadUsbAppViewWork);
    bad_usb_free(app->bad_usb_view);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Close records
    furi_record_close("gui");
    furi_record_close("notification");
    furi_record_close("dialogs");

    free(app);
}

int32_t bad_usb_app(void* p) {
    FuriHalUsbInterface* usb_mode_prev = furi_hal_usb_get_config();
    furi_hal_usb_set_config(&usb_hid);

    BadUsbApp* bad_usb_app = bad_usb_app_alloc();

    view_dispatcher_run(bad_usb_app->view_dispatcher);

    furi_hal_usb_set_config(usb_mode_prev);
    bad_usb_app_free(bad_usb_app);

    return 0;
}
