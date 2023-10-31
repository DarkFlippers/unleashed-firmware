#include "../infrared_app_i.h"

#include <string.h>
#include <toolbox/path.h>

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

    view_set_orientation(view_stack_get_view(infrared->view_stack), ViewOrientationHorizontal);
    view_stack_add_view(infrared->view_stack, text_input_get_view(infrared->text_input));

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewStack);
}

bool infrared_scene_edit_rename_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    InfraredRemote* remote = infrared->remote;
    SceneManager* scene_manager = infrared->scene_manager;
    InfraredAppState* app_state = &infrared->app_state;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypeTextEditDone) {
            bool success = false;
            const InfraredEditTarget edit_target = app_state->edit_target;
            if(edit_target == InfraredEditTargetButton) {
                const int32_t current_button_index = app_state->current_button_index;
                furi_assert(current_button_index != InfraredButtonIndexNone);
                infrared_show_loading_popup(infrared, true);
                success = infrared_remote_rename_signal(
                    remote, current_button_index, infrared->text_store[0]);
                infrared_show_loading_popup(infrared, false);
                app_state->current_button_index = InfraredButtonIndexNone;
            } else if(edit_target == InfraredEditTargetRemote) {
                success = infrared_rename_current_remote(infrared, infrared->text_store[0]);
            } else {
                furi_crash();
            }

            if(success) {
                scene_manager_next_scene(scene_manager, InfraredSceneEditRenameDone);
            } else {
                infrared_show_error_message(
                    infrared,
                    "Failed to\nrename %s",
                    edit_target == InfraredEditTargetButton ? "button" : "file");
                scene_manager_search_and_switch_to_previous_scene(
                    scene_manager, InfraredSceneRemoteList);
            }
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_edit_rename_on_exit(void* context) {
    InfraredApp* infrared = context;
    TextInput* text_input = infrared->text_input;

    view_stack_remove_view(infrared->view_stack, text_input_get_view(text_input));

    void* validator_context = text_input_get_validator_callback_context(text_input);
    text_input_set_validator(text_input, NULL, NULL);

    if(validator_context) {
        validator_is_file_free((ValidatorIsFile*)validator_context);
    }
}
