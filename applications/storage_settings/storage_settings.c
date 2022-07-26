#include "storage_settings.h"

static bool storage_settings_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    StorageSettings* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool storage_settings_back_event_callback(void* context) {
    furi_assert(context);
    StorageSettings* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static StorageSettings* storage_settings_alloc() {
    StorageSettings* app = malloc(sizeof(StorageSettings));

    app->gui = furi_record_open(RECORD_GUI);
    app->fs_api = furi_record_open(RECORD_STORAGE);
    app->notification = furi_record_open(RECORD_NOTIFICATION);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&storage_settings_scene_handlers, app);
    string_init(app->text_string);

    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, storage_settings_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, storage_settings_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, StorageSettingsViewSubmenu, submenu_get_view(app->submenu));

    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, StorageSettingsViewDialogEx, dialog_ex_get_view(app->dialog_ex));

    scene_manager_next_scene(app->scene_manager, StorageSettingsStart);

    return app;
}

static void storage_settings_free(StorageSettings* app) {
    view_dispatcher_remove_view(app->view_dispatcher, StorageSettingsViewSubmenu);
    submenu_free(app->submenu);

    view_dispatcher_remove_view(app->view_dispatcher, StorageSettingsViewDialogEx);
    dialog_ex_free(app->dialog_ex);

    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_NOTIFICATION);

    string_clear(app->text_string);

    free(app);
}

int32_t storage_settings_app(void* p) {
    UNUSED(p);
    StorageSettings* app = storage_settings_alloc();

    view_dispatcher_run(app->view_dispatcher);

    storage_settings_free(app);
    return 0;
}
