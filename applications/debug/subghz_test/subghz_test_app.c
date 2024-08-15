#include "subghz_test_app_i.h"

#include <furi.h>
#include <furi_hal.h>

static bool subghz_test_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SubGhzTestApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool subghz_test_app_back_event_callback(void* context) {
    furi_assert(context);
    SubGhzTestApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void subghz_test_app_tick_event_callback(void* context) {
    furi_assert(context);
    SubGhzTestApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

SubGhzTestApp* subghz_test_app_alloc(void) {
    SubGhzTestApp* app = malloc(sizeof(SubGhzTestApp));

    // GUI
    app->gui = furi_record_open(RECORD_GUI);

    // View Dispatcher
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&subghz_test_scene_handlers, app);

    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, subghz_test_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, subghz_test_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, subghz_test_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Open Notification record
    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    // SubMenu
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SubGhzTestViewSubmenu, submenu_get_view(app->submenu));

    // Widget
    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SubGhzTestViewWidget, widget_get_view(app->widget));

    // Popup
    app->popup = popup_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SubGhzTestViewPopup, popup_get_view(app->popup));

    // Carrier Test Module
    app->subghz_test_carrier = subghz_test_carrier_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubGhzTestViewCarrier,
        subghz_test_carrier_get_view(app->subghz_test_carrier));

    // Packet Test
    app->subghz_test_packet = subghz_test_packet_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubGhzTestViewPacket,
        subghz_test_packet_get_view(app->subghz_test_packet));

    // Static send
    app->subghz_test_static = subghz_test_static_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SubGhzTestViewStatic,
        subghz_test_static_get_view(app->subghz_test_static));

    scene_manager_next_scene(app->scene_manager, SubGhzTestSceneStart);

    return app;
}

void subghz_test_app_free(SubGhzTestApp* app) {
    furi_assert(app);

    // Submenu
    view_dispatcher_remove_view(app->view_dispatcher, SubGhzTestViewSubmenu);
    submenu_free(app->submenu);

    //  Widget
    view_dispatcher_remove_view(app->view_dispatcher, SubGhzTestViewWidget);
    widget_free(app->widget);

    // Popup
    view_dispatcher_remove_view(app->view_dispatcher, SubGhzTestViewPopup);
    popup_free(app->popup);

    // Carrier Test
    view_dispatcher_remove_view(app->view_dispatcher, SubGhzTestViewCarrier);
    subghz_test_carrier_free(app->subghz_test_carrier);

    // Packet Test
    view_dispatcher_remove_view(app->view_dispatcher, SubGhzTestViewPacket);
    subghz_test_packet_free(app->subghz_test_packet);

    // Static
    view_dispatcher_remove_view(app->view_dispatcher, SubGhzTestViewStatic);
    subghz_test_static_free(app->subghz_test_static);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Notifications
    furi_record_close(RECORD_NOTIFICATION);
    app->notifications = NULL;

    // Close records
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t subghz_test_app(void* p) {
    UNUSED(p);
    SubGhzTestApp* subghz_test_app = subghz_test_app_alloc();

    view_dispatcher_run(subghz_test_app->view_dispatcher);

    subghz_test_app_free(subghz_test_app);

    return 0;
}
