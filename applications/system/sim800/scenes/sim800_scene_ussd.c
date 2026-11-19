#include "../sim800_app_i.h"

void sim800_scene_ussd_on_enter(void* context) {
    Sim800App* app = context;

    if(app->text_store[0] != '*') strlcpy(app->text_store, "*100#", SIM800_USSD_MAX);

    sim800_text_input_reset(app->text_input);
    sim800_text_input_set_header_text(app->text_input, "USSD code");
    sim800_text_input_set_minimum_length(app->text_input, 2);
    sim800_text_input_set_result_callback(
        app->text_input, sim800_input_done_callback, app, app->text_store, SIM800_USSD_MAX, false);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextInput);
}

bool sim800_scene_ussd_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeCustom &&
       event.event == Sim800CustomEventTextInputDone) {
        scene_manager_next_scene(app->scene_manager, Sim800SceneUssdResult);
        return true;
    }
    return false;
}

void sim800_scene_ussd_on_exit(void* context) {
    Sim800App* app = context;
    sim800_text_input_reset(app->text_input);
}
