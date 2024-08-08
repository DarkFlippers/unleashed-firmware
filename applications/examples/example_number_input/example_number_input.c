#include "example_number_input.h"

bool example_number_input_custom_event_callback(void* context, uint32_t event) {
    furi_assert(context);
    ExampleNumberInput* app = context;
    return scene_manager_handle_custom_event(app->scene_manager, event);
}

static bool example_number_input_back_event_callback(void* context) {
    furi_assert(context);
    ExampleNumberInput* app = context;
    return scene_manager_handle_back_event(app->scene_manager);
}

static ExampleNumberInput* example_number_input_alloc() {
    ExampleNumberInput* app = malloc(sizeof(ExampleNumberInput));
    app->gui = furi_record_open(RECORD_GUI);

    app->view_dispatcher = view_dispatcher_alloc();

    app->scene_manager = scene_manager_alloc(&example_number_input_scene_handlers, app);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, example_number_input_custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, example_number_input_back_event_callback);

    app->number_input = number_input_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        ExampleNumberInputViewIdNumberInput,
        number_input_get_view(app->number_input));

    app->dialog_ex = dialog_ex_alloc();
    view_dispatcher_add_view(
        app->view_dispatcher,
        ExampleNumberInputViewIdShowNumber,
        dialog_ex_get_view(app->dialog_ex));

    app->current_number = 5;
    app->min_value = INT32_MIN;
    app->max_value = INT32_MAX;

    return app;
}

static void example_number_input_free(ExampleNumberInput* app) {
    furi_assert(app);

    view_dispatcher_remove_view(app->view_dispatcher, ExampleNumberInputViewIdShowNumber);
    dialog_ex_free(app->dialog_ex);

    view_dispatcher_remove_view(app->view_dispatcher, ExampleNumberInputViewIdNumberInput);
    number_input_free(app->number_input);

    scene_manager_free(app->scene_manager);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    app->gui = NULL;

    //Remove whatever is left
    free(app);
}

int32_t example_number_input(void* p) {
    UNUSED(p);
    ExampleNumberInput* app = example_number_input_alloc();

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    scene_manager_next_scene(app->scene_manager, ExampleNumberInputSceneShowNumber);

    view_dispatcher_run(app->view_dispatcher);

    example_number_input_free(app);

    return 0;
}
