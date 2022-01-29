#include "desktop_settings_app.h"
#include <furi.h>
#include "scenes/desktop_settings_scene.h"

static bool desktop_settings_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool desktop_settings_back_event_callback(void* context) {
    furi_assert(context);
    DesktopSettingsApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

DesktopSettingsApp* desktop_settings_app_alloc() {
    DesktopSettingsApp* app = furi_alloc(sizeof(DesktopSettingsApp));

    app->gui = furi_record_open("gui");
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&desktop_settings_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, desktop_settings_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, desktop_settings_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, DesktopSettingsAppViewMenu, submenu_get_view(app->submenu));

    app->code_input = code_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        DesktopSettingsAppViewPincodeInput,
        code_input_get_view(app->code_input));

    scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneStart);
    return app;
}

void desktop_settings_app_free(DesktopSettingsApp* app) {
    furi_assert(app);
    // Variable item list
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewMenu);
    submenu_free(app->submenu);
    view_dispatcher_remove_view(app->view_dispatcher, DesktopSettingsAppViewPincodeInput);
    code_input_free(app->code_input);
    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);
    // Records
    furi_record_close("gui");
    free(app);
}

extern int32_t desktop_settings_app(void* p) {
    DesktopSettingsApp* app = desktop_settings_app_alloc();
    LOAD_DESKTOP_SETTINGS(&app->settings);
    view_dispatcher_run(app->view_dispatcher);
    desktop_settings_app_free(app);
    return 0;
}
