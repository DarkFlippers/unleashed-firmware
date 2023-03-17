#include "signal_gen_app_i.h"

#include <furi.h>
#include <furi_hal.h>

static bool signal_gen_app_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    SignalGenApp* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool signal_gen_app_back_event_callback(void* context) {
    furi_assert(context);
    SignalGenApp* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static void signal_gen_app_tick_event_callback(void* context) {
    furi_assert(context);
    SignalGenApp* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

SignalGenApp* signal_gen_app_alloc() {
    SignalGenApp* app = malloc(sizeof(SignalGenApp));

    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();
    app->scene_manager = scene_manager_alloc(&signal_gen_scene_handlers, app);
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, signal_gen_app_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, signal_gen_app_back_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, signal_gen_app_tick_event_callback, 100);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    app->var_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        SignalGenViewVarItemList,
        variable_item_list_get_view(app->var_item_list));

    app->submenu = submenu_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SignalGenViewSubmenu, submenu_get_view(app->submenu));

    app->pwm_view = signal_gen_pwm_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher, SignalGenViewPwm, signal_gen_pwm_get_view(app->pwm_view));

    scene_manager_next_scene(app->scene_manager, SignalGenSceneStart);

    return app;
}

void signal_gen_app_free(SignalGenApp* app) {
    furi_assert(app);

    // Views
    view_dispatcher_remove_view(app->view_dispatcher, SignalGenViewVarItemList);
    view_dispatcher_remove_view(app->view_dispatcher, SignalGenViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, SignalGenViewPwm);

    submenu_free(app->submenu);
    variable_item_list_free(app->var_item_list);
    signal_gen_pwm_free(app->pwm_view);

    // View dispatcher
    view_dispatcher_free(app->view_dispatcher);
    scene_manager_free(app->scene_manager);

    // Close records
    furi_record_close(RECORD_GUI);

    free(app);
}

int32_t signal_gen_app(void* p) {
    UNUSED(p);
    SignalGenApp* signal_gen_app = signal_gen_app_alloc();

    view_dispatcher_run(signal_gen_app->view_dispatcher);

    signal_gen_app_free(signal_gen_app);

    return 0;
}
