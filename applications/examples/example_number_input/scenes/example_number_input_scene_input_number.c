#include "../example_number_input.h"

void example_number_input_scene_input_number_callback(void* context, int32_t number) {
    ExampleNumberInput* app = context;
    app->current_number = number;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

void example_number_input_scene_input_number_on_enter(void* context) {
    furi_assert(context);
    ExampleNumberInput* app = context;
    NumberInput* number_input = app->number_input;

    char str[50];
    snprintf(str, sizeof(str), "Set Number (%ld - %ld)", app->min_value, app->max_value);

    number_input_set_header_text(number_input, str);
    number_input_set_result_callback(
        number_input,
        example_number_input_scene_input_number_callback,
        context,
        app->current_number,
        app->min_value,
        app->max_value);

    view_dispatcher_switch_to_view(app->view_dispatcher, ExampleNumberInputViewIdNumberInput);
}

bool example_number_input_scene_input_number_on_event(void* context, SceneManagerEvent event) {
    ExampleNumberInput* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) { //Back button pressed
        scene_manager_previous_scene(app->scene_manager);
        return true;
    }
    return consumed;
}

void example_number_input_scene_input_number_on_exit(void* context) {
    UNUSED(context);
}
