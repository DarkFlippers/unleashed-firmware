#include "../infrared_app_i.h"

#include <dolphin/dolphin.h>

void infrared_scene_universal_save_name_on_enter(void* context) {
    InfraredApp* infrared = context;
    TextInput* text_input = infrared->text_input;

    // Offer the same generated name the learn flow would have picked, so the user can
    // just confirm it.
    FuriString* remote_name = furi_string_alloc_set(INFRARED_DEFAULT_REMOTE_NAME);
    infrared_find_vacant_remote_name(remote_name, INFRARED_APP_FOLDER);
    infrared_text_store_set(infrared, 0, "%s", furi_string_get_cstr(remote_name));
    furi_string_free(remote_name);

    text_input_set_header_text(text_input, "Name the remote");
    text_input_set_result_callback(
        text_input,
        infrared_text_input_callback,
        context,
        infrared->text_store[0],
        INFRARED_MAX_REMOTE_NAME_LENGTH,
        true);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewTextInput);
}

bool infrared_scene_universal_save_name_on_event(void* context, SceneManagerEvent event) {
    InfraredApp* infrared = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == InfraredCustomEventTypeTextEditDone) {
            const InfraredErrorCode error = infrared_add_named_remote_with_button(
                infrared,
                infrared->text_store[0],
                infrared->text_store[1],
                infrared->current_signal);

            if(!INFRARED_ERROR_PRESENT(error)) {
                dolphin_deed(DolphinDeedIrSave);
                scene_manager_next_scene(infrared->scene_manager, InfraredSceneUniversalSaveDone);
            } else {
                infrared_show_error_message(infrared, "Failed to\ncreate file");
                scene_manager_previous_scene(infrared->scene_manager);
            }
            consumed = true;
        }
    }

    return consumed;
}

void infrared_scene_universal_save_name_on_exit(void* context) {
    InfraredApp* infrared = context;
    text_input_reset(infrared->text_input);
}
