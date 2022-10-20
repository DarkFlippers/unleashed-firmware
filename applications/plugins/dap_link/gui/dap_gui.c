#include "dap_gui.h"
#include "dap_gui_i.h"

#define DAP_GUI_TICK 250

static bool dap_gui_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    DapGuiApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool dap_gui_back_event_callback(void* context) {
    furi_assert(context);
    DapGuiApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void dap_gui_tick_event_callback(void* context) {
    furi_assert(context);
    DapGuiApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

DapGuiApp* dap_gui_alloc() {
    DapGuiApp* app = malloc(sizeof(DapGuiApp));
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&dap_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(app->view_dispatcher, dap_gui_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, dap_gui_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, dap_gui_tick_event_callback, DAP_GUI_TICK);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        DapGuiAppViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    app->main_view = dap_main_view_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, DapGuiAppViewMainView, dap_main_view_get_view(app->main_view));

    app->widget = widget_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, DapGuiAppViewWidget, widget_get_view(app->widget));

    scene_manager_next_scene(app->scene_manager, DapSceneMain);

    return app;
}

void dap_gui_free(DapGuiApp* app) {
    view_dispatcher_remove_view(app->view_dispatcher, DapGuiAppViewVarItemList);
    variable_item_list_free(app->var_item_list);

    view_dispatcher_remove_view(app->view_dispatcher, DapGuiAppViewMainView);
    dap_main_view_free(app->main_view);

    view_dispatcher_remove_view(app->view_dispatcher, DapGuiAppViewWidget);
    widget_free(app->widget);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Close records
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);

    free(app);
}

int32_t dap_gui_thread(void* arg) {
    DapGuiApp* app = dap_gui_alloc();
    app->dap_app = arg;

    notification_message_block(app->notifications, &sequence_display_backlight_enforce_on);
    view_dispatcher_run(app->view_dispatcher);
    notification_message_block(app->notifications, &sequence_display_backlight_enforce_auto);

    dap_gui_free(app);
    return 0;
}