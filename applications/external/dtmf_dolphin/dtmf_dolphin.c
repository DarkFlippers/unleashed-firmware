#include "dtmf_dolphin_i.h"

#include <furi.h>
#include <furi_hal.h>

static bool dtmf_dolphin_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    DTMFDolphinApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool dtmf_dolphin_app_back_event_callback(void* context) {
    furi_assert(context);
    DTMFDolphinApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void dtmf_dolphin_app_tick_event_callback(void* context) {
    furi_assert(context);
    DTMFDolphinApp* app = context;

    scene_manager_handle_tick_event(app->scene_manager);
}

static DTMFDolphinApp* app_alloc() {
    DTMFDolphinApp* app = malloc(sizeof(DTMFDolphinApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&dtmf_dolphin_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, dtmf_dolphin_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, dtmf_dolphin_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, dtmf_dolphin_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->main_menu_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        DTMFDolphinViewMainMenu,
        variable_item_list_get_view(app->main_menu_list));

    app->dtmf_dolphin_dialer = dtmf_dolphin_dialer_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        DTMFDolphinViewDialer,
        dtmf_dolphin_dialer_get_view(app->dtmf_dolphin_dialer));

    app->notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message(app->notification, &sequence_display_backlight_enforce_on);

    scene_manager_next_scene(app->scene_manager, DTMFDolphinSceneStart);

    return app;
}

static void app_free(DTMFDolphinApp* app) {
    furi_assert(app);
    view_dispatcher_remove_view(app->view_dispatcher, DTMFDolphinViewMainMenu);
    view_dispatcher_remove_view(app->view_dispatcher, DTMFDolphinViewDialer);
    variable_item_list_free(app->main_menu_list);

    dtmf_dolphin_dialer_free(app->dtmf_dolphin_dialer);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    notification_message(app->notification, &sequence_display_backlight_enforce_auto);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    free(app);
}

int32_t dtmf_dolphin_app(void* p) {
    UNUSED(p);
    DTMFDolphinApp* app = app_alloc();

    view_dispatcher_run(app->view_dispatcher);

    app_free(app);
    return 0;
}