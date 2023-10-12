#include "gpio_app_i.h"

void gpio_scene_exit_confirm_dialog_callback(DialogExResult result, void* context) {
    GpioApp* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, result);
}

void gpio_scene_exit_confirm_on_enter(void* context) {
    GpioApp* app = context;
    DialogEx* dialog = app->dialog;

    dialog_ex_set_context(dialog, app);
    dialog_ex_set_left_button_text(dialog, "Exit");
    dialog_ex_set_right_button_text(dialog, "Stay");
    dialog_ex_set_header(dialog, "Exit USB-UART?", 22, 12, AlignLeft, AlignTop);
    dialog_ex_set_result_callback(dialog, gpio_scene_exit_confirm_dialog_callback);

    view_dispatcher_switch_to_view(app->view_dispatcher, GpioAppViewExitConfirm);
}

bool gpio_scene_exit_confirm_on_event(void* context, SceneManagerEvent event) {
    GpioApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultRight) {
            consumed = scene_manager_previous_scene(app->scene_manager);
        } else if(event.event == DialogExResultLeft) {
            scene_manager_search_and_switch_to_previous_scene(app->scene_manager, GpioSceneStart);
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = true;
    }

    return consumed;
}

void gpio_scene_exit_confirm_on_exit(void* context) {
    GpioApp* app = context;

    // Clean view
    dialog_ex_reset(app->dialog);
}
