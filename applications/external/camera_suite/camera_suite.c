#include "camera_suite.h"
#include <stdlib.h>

bool camera_suite_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    CameraSuite* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

void camera_suite_tick_event_callback(void* context) {
    furi_assert(context);
    CameraSuite* app = context;
    scene_manager_handle_tick_event(app->scene_manager);
}

//leave app if back button pressed
bool camera_suite_navigation_event_callback(void* context) {
    furi_assert(context);
    CameraSuite* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

CameraSuite* camera_suite_app_alloc() {
    CameraSuite* app = malloc(sizeof(CameraSuite));
    app->gui = furi_record_open(RECORD_GUI);
    app->notification = furi_record_open(RECORD_NOTIFICATION);

    //Turn backlight on, believe me this makes testing your app easier
    notification_message(app->notification, &sequence_display_backlight_on);

    //Scene additions
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);

    app->scene_manager = scene_manager_alloc(&camera_suite_scene_handlers, app);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, camera_suite_navigation_event_callback);
    view_dispatcher_set_tick_event_callback(
        app->view_dispatcher, camera_suite_tick_event_callback, 100);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, camera_suite_custom_event_callback);
    app->submenu = submenu_alloc();

    // Set defaults, in case no config loaded
    app->orientation = 0; // Orientation is "portrait", zero degrees by default.
    app->haptic = 1; // Haptic is on by default
    app->speaker = 1; // Speaker is on by default
    app->led = 1; // LED is on by default

    // Load configs
    camera_suite_read_settings(app);

    view_dispatcher_add_view(
        app->view_dispatcher, CameraSuiteViewIdMenu, submenu_get_view(app->submenu));

    app->camera_suite_view_start = camera_suite_view_start_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        CameraSuiteViewIdStartscreen,
        camera_suite_view_start_get_view(app->camera_suite_view_start));

    app->camera_suite_view_style_1 = camera_suite_view_style_1_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        CameraSuiteViewIdScene1,
        camera_suite_view_style_1_get_view(app->camera_suite_view_style_1));

    app->camera_suite_view_style_2 = camera_suite_view_style_2_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        CameraSuiteViewIdScene2,
        camera_suite_view_style_2_get_view(app->camera_suite_view_style_2));

    app->camera_suite_view_guide = camera_suite_view_guide_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        CameraSuiteViewIdGuide,
        camera_suite_view_guide_get_view(app->camera_suite_view_guide));

    app->button_menu = button_menu_alloc();

    app->variable_item_list = variable_item_list_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        CameraSuiteViewIdSettings,
        variable_item_list_get_view(app->variable_item_list));

    //End Scene Additions

    return app;
}

void camera_suite_app_free(CameraSuite* app) {
    furi_assert(app);

    // Scene manager
    scene_manager_free(app->scene_manager);

    // View Dispatcher
    view_dispatcher_remove_view(app->view_dispatcher, CameraSuiteViewIdMenu);
    view_dispatcher_remove_view(app->view_dispatcher, CameraSuiteViewIdScene1);
    view_dispatcher_remove_view(app->view_dispatcher, CameraSuiteViewIdScene2);
    view_dispatcher_remove_view(app->view_dispatcher, CameraSuiteViewIdGuide);
    view_dispatcher_remove_view(app->view_dispatcher, CameraSuiteViewIdSettings);
    submenu_free(app->submenu);

    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);

    // Free remaining resources
    camera_suite_view_start_free(app->camera_suite_view_start);
    camera_suite_view_style_1_free(app->camera_suite_view_style_1);
    camera_suite_view_style_2_free(app->camera_suite_view_style_2);
    camera_suite_view_guide_free(app->camera_suite_view_guide);
    button_menu_free(app->button_menu);
    variable_item_list_free(app->variable_item_list);

    app->gui = NULL;
    app->notification = NULL;

    //Remove whatever is left
    free(app);
}

/** Main entry point for initialization. */
int32_t camera_suite_app(void* p) {
    UNUSED(p);
    CameraSuite* app = camera_suite_app_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    // Init with start scene.
    scene_manager_next_scene(app->scene_manager, CameraSuiteSceneStart);
    furi_hal_power_suppress_charge_enter();
    view_dispatcher_run(app->view_dispatcher);
    camera_suite_save_settings(app);
    furi_hal_power_suppress_charge_exit();
    camera_suite_app_free(app);
    return 0;
}
