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

BtSettingsApp* bt_settings_app_alloc(void) {
    BtSettingsApp* app = malloc(sizeof(BtSettingsApp));

    app->gui = furi_record_open(RECORD_GUI);
    app->bt = furi_record_open(RECORD_BT);

    // View Dispatcher and Scene Manager
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&bt_settings_scene_handlers, app);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, bt_settings_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, bt_settings_back_event_callback);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Gui Modules
    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        BtSettingsAppViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    app->dialog = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BtSettingsAppViewDialog, dialog_ex_get_view(app->dialog));

    app->popup = popup_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, BtSettingsAppViewPopup, popup_get_view(app->popup));

    bt_get_settings(app->bt, &app->settings);

    // Set first scene
    scene_manager_next_scene(app->scene_manager, BtSettingsAppSceneStart);
    return app;
}

void bt_settings_app_free(BtSettingsApp* app) {
    furi_assert(app);
    bt_set_settings(app->bt, &app->settings);
    // Gui modules
    view_dispatcher_remove_view(app->view_dispatcher, BtSettingsAppViewVarItemList);
    variable_item_list_free(app->var_item_list);

    view_dispatcher_remove_view(app->view_dispatcher, BtSettingsAppViewDialog);
    dialog_ex_free(app->dialog);

    view_dispatcher_remove_view(app->view_dispatcher, BtSettingsAppViewPopup);
    popup_free(app->popup);

    // View Dispatcher and Scene Manager
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_BT);
    free(app);
}

extern int32_t bt_settings_app(void* p) {
    UNUSED(p);
    BtSettingsApp* app = bt_settings_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    bt_settings_app_free(app);
    return 0;
}
