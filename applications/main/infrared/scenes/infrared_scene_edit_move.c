#include "../infrared_i.h"

static void infrared_scene_edit_move_dialog_result_callback(DialogExResult result, void* context) {
    Infrared* infrared = context;
    view_dispatcher_send_custom_event(infrared->view_dispatcher, result);
}

void infrared_scene_edit_move_on_enter(void* context) {
    Infrared* infrared = context;
    DialogEx* dialog_ex = infrared->dialog_ex;
    InfraredRemote* remote = infrared->remote;

    const InfraredEditTarget edit_target = infrared->app_state.edit_target;
    if(edit_target == InfraredEditTargetButton) {
        int32_t current_button_index = infrared->app_state.current_button_index_move_orig;
        furi_assert(current_button_index != InfraredButtonIndexNone);

        dialog_ex_set_header(dialog_ex, "Move Button?", 64, 0, AlignCenter, AlignTop);
        InfraredRemoteButton* current_button =
            infrared_remote_get_button(remote, current_button_index);
        InfraredSignal* signal = infrared_remote_button_get_signal(current_button);

        if(infrared_signal_is_raw(signal)) {
            const InfraredRawSignal* raw = infrared_signal_get_raw_signal(signal);
            infrared_text_store_set(
                infrared,
                0,
                "%s\nRAW\n%ld samples",
                infrared_remote_button_get_name(current_button),
                raw->timings_size);

        } else {
            const InfraredMessage* message = infrared_signal_get_message(signal);
            infrared_text_store_set(
                infrared,
                0,
                "%s\n%s\nA=0x%0*lX C=0x%0*lX",
                infrared_remote_button_get_name(current_button),
                infrared_get_protocol_name(message->protocol),
                ROUND_UP_TO(infrared_get_protocol_address_length(message->protocol), 4),
                message->address,
                ROUND_UP_TO(infrared_get_protocol_command_length(message->protocol), 4),
                message->command);
        }
    } else {
        furi_assert(0);
    }

    dialog_ex_set_text(dialog_ex, infrared->text_store[0], 64, 31, AlignCenter, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
    dialog_ex_set_left_button_text(dialog_ex, "Cancel");
    dialog_ex_set_right_button_text(dialog_ex, "Move");
    dialog_ex_set_result_callback(dialog_ex, infrared_scene_edit_move_dialog_result_callback);
    dialog_ex_set_context(dialog_ex, context);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewDialogEx);
}

bool infrared_scene_edit_move_on_event(void* context, SceneManagerEvent event) {
    Infrared* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            scene_manager_previous_scene(scene_manager);
            consumed = true;
        } else if(event.event == DialogExResultRight) {
            bool success = false;
            InfraredRemote* remote = infrared->remote;
            InfraredAppState* app_state = &infrared->app_state;
            const InfraredEditTarget edit_target = app_state->edit_target;

            if(edit_target == InfraredEditTargetButton) {
                furi_assert(app_state->current_button_index != InfraredButtonIndexNone);
                success = infrared_remote_move_button(
                    remote,
                    app_state->current_button_index_move_orig,
                    app_state->current_button_index);
                app_state->current_button_index_move_orig = InfraredButtonIndexNone;
                app_state->current_button_index = InfraredButtonIndexNone;
            } else {
                furi_assert(0);
            }

            if(success) {
                scene_manager_next_scene(scene_manager, InfraredSceneEditMoveDone);
            } else {
                const uint32_t possible_scenes[] = {InfraredSceneRemoteList, InfraredSceneStart};
                scene_manager_search_and_switch_to_previous_scene_one_of(
                    scene_manager, possible_scenes, COUNT_OF(possible_scenes));
            }
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_edit_move_on_exit(void* context) {
    Infrared* infrared = context;
    UNUSED(infrared);
}
