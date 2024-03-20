#include "../infrared_app_i.h"

#include <string.h>
#include <toolbox/path.h>

static int32_t infrared_scene_edit_rename_task_callback(void* context) {
    InfraredApp* infrared = context;
    InfraredAppState* app_state = &infrared->app_state;
    const InfraredEditTarget edit_target = app_state->edit_target;

    bool success;
    if(edit_target == InfraredEditTargetButton) {
        furi_assert(app_state->current_button_index != InfraredButtonIndexNone);
        success = infrared_remote_rename_signal(
            infrared->remote, app_state->current_button_index, infrared->text_store[0]);
    } else if(edit_target == InfraredEditTargetRemote) {
        success = infrared_rename_current_remote(infrared, infrared->text_store[0]);
    } else {
        furi_crash();
    }

    view_dispatcher_send_custom_event(
        infrared->view_dispatcher, InfraredCustomEventTypeTaskFinished);

    return success;
}

void infrared_scene_edit_rename_on_enter(void* context) {
    InfraredApp* infrared = context;
    InfraredRemote* remote = infrared->remote;
    TextInput* text_input = infrared->text_input;
    size_t enter_name_length = 0;

    const InfraredEditTarget edit_target = infrared->app_state.edit_target;
    if(edit_target == InfraredEditTargetButton) {
        text_input_set_header_text(text_input, "Name the button");

        const int32_t current_button_index = infrared->app_state.current_button_index;
        furi_check(current_button_index != InfraredButtonIndexNone);

        enter_name_length = INFRARED_MAX_BUTTON_NAME_LENGTH;
        strncpy(
            infrared->text_store[0],
            infrared_remote_get_signal_name(remote, current_button_index),
            enter_name_length);

    } else if(edit_target == InfraredEditTargetRemote) {
        text_input_set_header_text(text_input, "Name the remote");
        enter_name_length = INFRARED_MAX_REMOTE_NAME_LENGTH;
        strncpy(infrared->text_store[0], infrared_remote_get_name(remote), enter_name_length);

        FuriString* folder_path;
        folder_path = furi_string_alloc();

        if(furi_string_end_with(infrared->file_path, INFRARED_APP_EXTENSION)) {
            path_extract_dirname(furi_string_get_cstr(infrared->file_path), folder_path);
        }

        ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
            furi_string_get_cstr(folder_path),
            INFRARED_APP_EXTENSION,
            infrared_remote_get_name(remote));
        text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

        furi_string_free(folder_path);
    } else {
        furi_crash();
    }

    text_input_set_result_callback(
        text_input,
        infrared_text_input_callback,
        context,
        infrared->text_store[0],
        enter_name_length,
        false);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewTextInput);
}

bool infrared_scene_edit_rename_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    SceneManager* scene_manager = infrared->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypeTextEditDone) {
            // Rename a button or a remote in a separate thread
            infrared_blocking_task_start(infrared, infrared_scene_edit_rename_task_callback);

        } else if(event.event == InfraredCustomEventTypeTaskFinished) {
            const bool task_success = infrared_blocking_task_finalize(infrared);
            InfraredAppState* app_state = &infrared->app_state;

            if(task_success) {
                scene_manager_next_scene(scene_manager, InfraredSceneEditRenameDone);
            } else {
                const char* edit_target_text =
                    app_state->edit_target == InfraredEditTargetButton ? "button" : "file";
                infrared_show_error_message(infrared, "Failed to\nrename %s", edit_target_text);
                scene_manager_search_and_switch_to_previous_scene(
                    scene_manager, InfraredSceneRemoteList);
            }

            app_state->current_button_index = InfraredButtonIndexNone;
        }
        consumed = true;
    }

    return consumed;
}

void infrared_scene_edit_rename_on_exit(void* context) {
    InfraredApp* infrared = context;
    TextInput* text_input = infrared->text_input;

    ValidatorIsFile* validator_context = text_input_get_validator_callback_context(text_input);
    if(validator_context) {
        validator_is_file_free(validator_context);
    }

    text_input_reset(text_input);
}
