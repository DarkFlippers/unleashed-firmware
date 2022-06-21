#include "../infrared_i.h"

#include <string.h>
#include <toolbox/path.h>

void infrared_scene_edit_rename_on_enter(void* context) {
    Infrared* infrared = context;
    InfraredRemote* remote = infrared->remote;
    TextInput* text_input = infrared->text_input;
    size_t enter_name_length = 0;

    const InfraredEditTarget edit_target = infrared->app_state.edit_target;
    if(edit_target == InfraredEditTargetButton) {
        text_input_set_header_text(text_input, "Name the button");

        const int32_t current_button_index = infrared->app_state.current_button_index;
        furi_assert(current_button_index != InfraredButtonIndexNone);

        InfraredRemoteButton* current_button =
            infrared_remote_get_button(remote, current_button_index);
        enter_name_length = INFRARED_MAX_BUTTON_NAME_LENGTH;
        strncpy(
            infrared->text_store[0],
            infrared_remote_button_get_name(current_button),
            enter_name_length);

    } else if(edit_target == InfraredEditTargetRemote) {
        text_input_set_header_text(text_input, "Name the remote");
        enter_name_length = INFRARED_MAX_REMOTE_NAME_LENGTH;
        strncpy(infrared->text_store[0], infrared_remote_get_name(remote), enter_name_length);

        string_t folder_path;
        string_init(folder_path);

        if(string_end_with_str_p(infrared->file_path, INFRARED_APP_EXTENSION)) {
            path_extract_dirname(string_get_cstr(infrared->file_path), folder_path);
        }

        ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
            string_get_cstr(folder_path),
            INFRARED_APP_EXTENSION,
            infrared_remote_get_name(remote));
        text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

        string_clear(folder_path);
    } else {
        furi_assert(0);
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
    Infrared* infrared = context;
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
                success = infrared_remote_rename_button(
                    remote, infrared->text_store[0], current_button_index);
                app_state->current_button_index = InfraredButtonIndexNone;
            } else if(edit_target == InfraredEditTargetRemote) {
                success = infrared_rename_current_remote(infrared, infrared->text_store[0]);
            } else {
                furi_assert(0);
            }

            if(success) {
                scene_manager_next_scene(scene_manager, InfraredSceneEditRenameDone);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    scene_manager, InfraredSceneRemoteList);
            }
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_edit_rename_on_exit(void* context) {
    Infrared* infrared = context;
    TextInput* text_input = infrared->text_input;

    void* validator_context = text_input_get_validator_callback_context(text_input);
    text_input_set_validator(text_input, NULL, NULL);

    if(validator_context) {
        validator_is_file_free((ValidatorIsFile*)validator_context);
    }
}
