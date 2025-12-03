#include "example_date_time_input.h"

bool example_date_time_input_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    ExampleDateTimeInput* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool example_date_time_input_back_event_callback(void* context) {
    furi_assert(context);
    ExampleDateTimeInput* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static ExampleDateTimeInput* example_date_time_input_alloc() {
    ExampleDateTimeInput* app = malloc(sizeof(ExampleDateTimeInput));
    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();

    app->scene_manager = scene_manager_alloc(&example_date_time_input_scene_handlers, app);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, example_date_time_input_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, example_date_time_input_back_event_callback);

    app->date_time_input = date_time_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        ExampleDateTimeInputViewIdDateTimeInput,
        date_time_input_get_view(app->date_time_input));

    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        ExampleDateTimeInputViewIdShowDateTime,
        dialog_ex_get_view(app->dialog_ex));

    // Fill in current date & time
    furi_hal_rtc_get_datetime(&app->date_time);
    app->edit_date = false;
    app->edit_time = false;

    return app;
}

static void example_date_time_input_free(ExampleDateTimeInput* app) {
    furi_assert(app);

    view_dispatcher_remove_view(app->view_dispatcher, ExampleDateTimeInputViewIdShowDateTime);
    dialog_ex_free(app->dialog_ex);

    view_dispatcher_remove_view(app->view_dispatcher, ExampleDateTimeInputViewIdDateTimeInput);
    date_time_input_free(app->date_time_input);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    free(app);
}

int32_t example_date_time_input(void* p) {
    UNUSED(p);
    ExampleDateTimeInput* app = example_date_time_input_alloc();

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    scene_manager_next_scene(app->scene_manager, ExampleDateTimeInputSceneShowDateTime);

    view_dispatcher_run(app->view_dispatcher);

    example_date_time_input_free(app);

    return 0;
}
