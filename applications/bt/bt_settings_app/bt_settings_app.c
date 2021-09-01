#include "bt_settings_app.h"

static bool bt_settings_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    BtSettingsApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool bt_settings_back_event_callback(void* context) {
    furi_assert(context);
    BtSettingsApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

BtSettingsApp* bt_settings_app_alloc() {
    BtSettingsApp* app = furi_alloc(sizeof(BtSettingsApp));

    // Load settings
    bt_settings_load(&app->settings);
    app->gui = furi_record_open("gui");

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&bt_settings_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, bt_settings_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, bt_settings_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BtSettingsAppViewSubmenu, submenu_get_view(app->submenu));
    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BtSettingsAppViewDialogEx, dialog_ex_get_view(app->dialog_ex));

    scene_manager_next_scene(app->scene_manager, BtSettingsAppSceneStart);
    return app;
}

void bt_settings_app_free(BtSettingsApp* app) {
    furi_assert(app);
    // Submenu
    view_dispatcher_remove_view(app->view_dispatcher, BtSettingsAppViewSubmenu);
    submenu_free(app->submenu);
    // Dialog
    view_dispatcher_remove_view(app->view_dispatcher, BtSettingsAppViewDialogEx);
    dialog_ex_free(app->dialog_ex);
    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    // Records
    furi_record_close("gui");
    free(app);
}

extern int32_t bt_settings_app(void* p) {
    BtSettingsApp* app = bt_settings_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    bt_settings_save(&app->settings);
    bt_settings_app_free(app);
    return 0;
}
