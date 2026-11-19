#include "../sim800_app_i.h"

void sim800_scene_sms_text_on_enter(void* context) {
    Sim800App* app = context;

    strlcpy(app->text_store, furi_string_get_cstr(app->sms_text), SIM800_SMS_TEXT_MAX);

    sim800_text_input_reset(app->text_input);
    sim800_text_input_set_header_text(app->text_input, "Message text");
    sim800_text_input_set_minimum_length(app->text_input, 1);
    sim800_text_input_set_result_callback(
        app->text_input,
        sim800_input_done_callback,
        app,
        app->text_store,
        SIM800_SMS_TEXT_MAX,
        false);

    // The result scene only sends on a fresh entry, so arm it here
    scene_manager_set_scene_state(app->scene_manager, Sim800SceneSmsResult, 0);

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewTextInput);
}

bool sim800_scene_sms_text_on_event(void* context, SceneManagerEvent event) {
    Sim800App* app = context;

    if(event.type == SceneManagerEventTypeCustom &&
       event.event == Sim800CustomEventTextInputDone) {
        furi_string_set_str(app->sms_text, app->text_store);
        scene_manager_next_scene(app->scene_manager, Sim800SceneSmsResult);
        return true;
    }
    return false;
}

void sim800_scene_sms_text_on_exit(void* context) {
    Sim800App* app = context;
    sim800_text_input_reset(app->text_input);
}
