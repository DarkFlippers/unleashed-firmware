#include "../example_date_time_input.h"

void example_date_time_input_scene_input_date_time_callback(void* context) {
    ExampleDateTimeInput* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

void example_date_time_input_scene_input_date_time_on_enter(void* context) {
    furi_assert(context);
    ExampleDateTimeInput* app = context;
    DateTimeInput* date_time_input = app->date_time_input;

    date_time_input_set_result_callback(
        date_time_input,
        NULL,
        example_date_time_input_scene_input_date_time_callback,
        context,
        &app->date_time);

    date_time_input_set_editable_fields(
        date_time_input,

        app->edit_date,
        app->edit_date,
        app->edit_date,

        app->edit_time,
        app->edit_time,
        app->edit_time);

    view_dispatcher_switch_to_view(app->view_dispatcher, ExampleDateTimeInputViewIdDateTimeInput);
}

bool example_date_time_input_scene_input_date_time_on_event(void* context, SceneManagerEvent event) {
    ExampleDateTimeInput* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) { //Back button pressed
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return consumed;
}

void example_date_time_input_scene_input_date_time_on_exit(void* context) {
    UNUSED(context);
}
