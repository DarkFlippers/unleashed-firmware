#include "../infrared_app_i.h"

static void
    infrared_scene_edit_delete_dialog_result_callback(DialogExResult result, void* context) {
    InfraredApp* infrared = context;
    view_dispatcher_send_custom_event(infrared->view_dispatcher, result);
}

static int32_t infrared_scene_edit_delete_task_callback(void* context) {
    InfraredApp* infrared = context;
    InfraredAppState* app_state = &infrared->app_state;
    const InfraredEditTarget edit_target = app_state->edit_target;

    InfraredErrorCode error = InfraredErrorCodeNone;
    if(edit_target == InfraredEditTargetButton) {
        furi_assert(app_state->current_button_index != InfraredButtonIndexNone);
        error = infrared_remote_delete_signal(infrared->remote, app_state->current_button_index);
    } else if(edit_target == InfraredEditTargetRemote) {
        error = infrared_remote_remove(infrared->remote);
    } else {
        furi_crash();
    }

    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypeTaskFinished);

    return error;
}

void infrared_scene_edit_delete_on_enter(void* context) {
    InfraredApp* infrared = context;
    DialogEx* dialog_ex = infrared->dialog_ex;
    InfraredRemote* remote = infrared->remote;
    const InfraredEditTarget edit_target = infrared->app_state.edit_target;

    if(edit_target == InfraredEditTargetButton) {
        dialog_ex_set_header(dialog_ex, "Delete Button?", 64, 0, AlignCenter, AlignTop);

        const int32_t current_button_index = infrared->app_state.current_button_index;
        furi_check(current_button_index != InfraredButtonIndexNone);

        InfraredErrorCode error =
            infrared_remote_load_signal(remote, infrared->current_signal, current_button_index);
        if(INFRARED_ERROR_PRESENT(error)) {
            const char* format =
                (INFRARED_ERROR_CHECK(error, InfraredErrorCodeSignalRawUnableToReadTooLongData)) ?
                    "Failed to delete\n\"%s\" is too long.\nTry to edit file from pc" :
                    "Failed to load\n\"%s\"";
            infrared_show_error_message(
                infrared, format, infrared_remote_get_signal_name(remote, current_button_index));
            scene_manager_previous_scene(infrared->scene_manager);
            return;
        }

        if(infrared_signal_is_raw(infrared->current_signal)) {
            const InfraredRawSignal* raw =
                infrared_signal_get_raw_signal(infrared->current_signal);
            infrared_text_store_set(
                infrared,
                0,
                "%s\nRAW\n%zu samples",
                infrared_remote_get_signal_name(remote, current_button_index),
                raw->timings_size);

        } else {
            const InfraredMessage* message = infrared_signal_get_message(infrared->current_signal);
            infrared_text_store_set(
                infrared,
                0,
                "%s\n%s\nA=0x%0*lX C=0x%0*lX",
                infrared_remote_get_signal_name(remote, current_button_index),
                infrared_get_protocol_name(message->protocol),
                ROUND_UP_TO(infrared_get_protocol_address_length(message->protocol), 4),
                message->address,
                ROUND_UP_TO(infrared_get_protocol_command_length(message->protocol), 4),
                message->command);
        }

    } else if(edit_target == InfraredEditTargetRemote) {
        dialog_ex_set_header(dialog_ex, "Delete Remote?", 64, 0, AlignCenter, AlignTop);
        infrared_text_store_set(
            infrared,
            0,
            "%s\n with %zu buttons",
            infrared_remote_get_name(remote),
            infrared_remote_get_signal_count(remote));
    } else {
        furi_crash();
    }

    dialog_ex_set_text(dialog_ex, infrared->text_store[0], 64, 31, AlignCenter, AlignCenter);
    dialog_ex_set_icon(dialog_ex, 0, 0, NULL);
    dialog_ex_set_left_button_text(dialog_ex, "Cancel");
    dialog_ex_set_right_button_text(dialog_ex, "Delete");
    dialog_ex_set_result_callback(dialog_ex, infrared_scene_edit_delete_dialog_result_callback);
    dialog_ex_set_context(dialog_ex, context);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewDialogEx);
}

bool infrared_scene_edit_delete_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DialogExResultLeft) {
            scene_manager_previous_scene(scene_manager);
        } else if(event.event == DialogExResultRight) {
            // Delete a button or a remote in a separate thread
            infrared_blocking_task_start(infrared, infrared_scene_edit_delete_task_callback);

        } else if(event.event == InfraredCustomEventTypeTaskFinished) {
            const InfraredErrorCode task_error = infrared_blocking_task_finalize(infrared);

            InfraredAppState* app_state = &infrared->app_state;

            if(!INFRARED_ERROR_PRESENT(task_error)) {
                scene_manager_next_scene(scene_manager, InfraredSceneEditDeleteDone);
            } else {
                if(INFRARED_ERROR_CHECK(
                       task_error, InfraredErrorCodeSignalRawUnableToReadTooLongData)) {
                    const uint8_t index = INFRARED_ERROR_GET_INDEX(task_error);
                    const char* format =
                        "Failed to delete\n\"%s\" is too long.\nTry to edit file from pc";
                    infrared_show_error_message(
                        infrared,
                        format,
                        infrared_remote_get_signal_name(infrared->remote, index));
                } else {
                    const char* edit_target_text =
                        app_state->edit_target == InfraredEditTargetButton ? "button" : "file";
                    infrared_show_error_message(
                        infrared, "Failed to\ndelete %s", edit_target_text);
                }

                const uint32_t possible_scenes[] = {InfraredSceneRemoteList, InfraredSceneRemote};
                scene_manager_search_and_switch_to_previous_scene_one_of(
                    scene_manager, possible_scenes, COUNT_OF(possible_scenes));
            }

            app_state->current_button_index = InfraredButtonIndexNone;
        }
        consumed = true;
    }

    return consumed;
}

void infrared_scene_edit_delete_on_exit(void* context) {
    InfraredApp* infrared = context;
    dialog_ex_reset(infrared->dialog_ex);
}
