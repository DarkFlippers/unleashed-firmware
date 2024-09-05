#include "../example_number_input.h"

static void
    example_number_input_scene_confirm_dialog_callback(DialogExResult result, void* context) {
    ExampleNumberInput* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

static void example_number_input_scene_update_view(void* context) {
    ExampleNumberInput* app = context;
    DialogEx* dialog_ex = app->dialog_ex;

    dialog_ex_set_header(dialog_ex, "The number is", 64, 0, AlignCenter, AlignTop);

    char buffer[12] = {};
    snprintf(buffer, sizeof(buffer), "%ld", app->current_number);
    dialog_ex_set_text(dialog_ex, buffer, 64, 29, AlignCenter, AlignCenter);

    dialog_ex_set_left_button_text(dialog_ex, "Min");
    dialog_ex_set_right_button_text(dialog_ex, "Max");
    dialog_ex_set_center_button_text(dialog_ex, "Change");

    dialog_ex_set_result_callback(dialog_ex, example_number_input_scene_confirm_dialog_callback);
    dialog_ex_set_context(dialog_ex, app);
}

void example_number_input_scene_show_number_on_enter(void* context) {
    furi_assert(context);
    ExampleNumberInput* app = context;

    example_number_input_scene_update_view(app);

    view_dispatcher_switch_to_view(app->view_dispatcher, ExampleNumberInputViewIdShowNumber);
}

bool example_number_input_scene_show_number_on_event(void* context, SceneManagerEvent event) {
    ExampleNumberInput* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DialogExResultCenter:
            scene_manager_next_scene(app->scene_manager, ExampleNumberInputSceneInputNumber);
            consumed = true;
            break;
        case DialogExResultLeft:
            scene_manager_next_scene(app->scene_manager, ExampleNumberInputSceneInputMin);
            consumed = true;
            break;
        case DialogExResultRight:
            scene_manager_next_scene(app->scene_manager, ExampleNumberInputSceneInputMax);
            consumed = true;
            break;
        default:
            break;
        }
    }

    return consumed;
}

void example_number_input_scene_show_number_on_exit(void* context) {
    UNUSED(context);
}
