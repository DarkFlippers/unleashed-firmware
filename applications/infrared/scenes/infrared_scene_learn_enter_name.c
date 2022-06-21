#include "../infrared_i.h"

void infrared_scene_learn_enter_name_on_enter(void* context) {
    Infrared* infrared = context;
    TextInput* text_input = infrared->text_input;
    InfraredSignal* signal = infrared->received_signal;

    if(infrared_signal_is_raw(signal)) {
        InfraredRawSignal* raw = infrared_signal_get_raw_signal(signal);
        infrared_text_store_set(infrared, 0, "RAW_%d", raw->timings_size);
    } else {
        InfraredMessage* message = infrared_signal_get_message(signal);
        infrared_text_store_set(
            infrared,
            0,
            "%.4s_%0*lX",
            infrared_get_protocol_name(message->protocol),
            ROUND_UP_TO(infrared_get_protocol_command_length(message->protocol), 4),
            message->command);
    }

    text_input_set_header_text(text_input, "Name the button");
    text_input_set_result_callback(
        text_input,
        infrared_text_input_callback,
        context,
        infrared->text_store[0],
        INFRARED_MAX_BUTTON_NAME_LENGTH,
        true);
    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewTextInput);
}

bool infrared_scene_learn_enter_name_on_event(void* context, SceneManagerEvent event) {
    Infrared* infrared = context;
    InfraredSignal* signal = infrared->received_signal;
    SceneManager* scene_manager = infrared->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypeTextEditDone) {
            bool success = false;
            if(infrared->app_state.is_learning_new_remote) {
                success =
                    infrared_add_remote_with_button(infrared, infrared->text_store[0], signal);
            } else {
                success =
                    infrared_remote_add_button(infrared->remote, infrared->text_store[0], signal);
            }

            if(success) {
                scene_manager_next_scene(scene_manager, InfraredSceneLearnDone);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    scene_manager, InfraredSceneRemoteList);
            }
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_learn_enter_name_on_exit(void* context) {
    Infrared* infrared = context;
    UNUSED(infrared);
}
