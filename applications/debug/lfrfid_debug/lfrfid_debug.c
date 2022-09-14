#include "lfrfid_debug_i.h"

static bool lfrfid_debug_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    LfRfidDebug* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool lfrfid_debug_back_event_callback(void* context) {
    furi_assert(context);
    LfRfidDebug* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static LfRfidDebug* lfrfid_debug_alloc() {
    LfRfidDebug* app = malloc(sizeof(LfRfidDebug));

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&lfrfid_debug_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, lfrfid_debug_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, lfrfid_debug_back_event_callback);

    // Open GUI record
    app->gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Submenu
    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, LfRfidDebugViewSubmenu, submenu_get_view(app->submenu));

    // Tune view
    app->tune_view = lfrfid_debug_view_tune_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        LfRfidDebugViewTune,
        lfrfid_debug_view_tune_get_view(app->tune_view));

    return app;
}

static void lfrfid_debug_free(LfRfidDebug* app) {
    furi_assert(app);

    // Submenu
    view_dispatcher_remove_view(app->view_dispatcher, LfRfidDebugViewSubmenu);
    submenu_free(app->submenu);

    // Tune view
    view_dispatcher_remove_view(app->view_dispatcher, LfRfidDebugViewTune);
    lfrfid_debug_view_tune_free(app->tune_view);

    // View Dispatcher
    view_dispatcher_free(app->view_dispatcher);

    // Scene Manager
    scene_manager_free(app->scene_manager);

    // GUI
    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    free(app);
}

int32_t lfrfid_debug_app(void* p) {
    UNUSED(p);
    LfRfidDebug* app = lfrfid_debug_alloc();

    scene_manager_next_scene(app->scene_manager, LfRfidDebugSceneStart);

    view_dispatcher_run(app->view_dispatcher);

    lfrfid_debug_free(app);

    return 0;
}