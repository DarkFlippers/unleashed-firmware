#include "../subbrute_i.h"
#include "subbrute_scene.h"
#include <lib/toolbox/random_name.h>

#define TAG "SubBruteSceneSaveFile"

void subbrute_scene_save_name_on_enter(void* context) {
    SubBruteState* instance = (SubBruteState*)context;

    // Setup view
    TextInput* text_input = instance->text_input;
    set_random_name(instance->text_store, sizeof(instance->text_store));

    text_input_set_header_text(text_input, "Name of file");
    text_input_set_result_callback(
        text_input,
        subbrute_text_input_callback,
        instance,
        instance->text_store,
        SUBBRUTE_MAX_LEN_NAME,
        true);

    furi_string_reset(instance->file_path);
    furi_string_set_str(instance->file_path, SUBBRUTE_PATH);

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        furi_string_get_cstr(instance->file_path), SUBBRUTE_FILE_EXT, "");
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(instance->view_dispatcher, SubBruteViewTextInput);
}

bool subbrute_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(instance->scene_manager);
        return true;
    } else if(
        event.type == SceneManagerEventTypeCustom &&
        event.event == SubBruteCustomEventTypeTextEditDone) {
#ifdef FURI_DEBUG
        FURI_LOG_D(TAG, "Saving: %s", instance->text_store);
#endif
        bool success = false;
        if(strcmp(instance->text_store, "")) {
            furi_string_reset(instance->file_path);
            furi_string_cat_printf(
                instance->file_path,
                "%s/%s%s",
                SUBBRUTE_PATH,
                instance->text_store,
                SUBBRUTE_FILE_EXT);

            if(subbrute_device_save_file(
                   instance->device, furi_string_get_cstr(instance->file_path))) {
                scene_manager_next_scene(instance->scene_manager, SubBruteSceneSaveSuccess);
                success = true;
                consumed = true;
            }
        }

        if(!success) {
            dialog_message_show_storage_error(instance->dialogs, "Error during saving!");
            consumed = scene_manager_search_and_switch_to_previous_scene(
                instance->scene_manager, SubBruteSceneSetupAttack);
        }
    }
    return consumed;
}

void subbrute_scene_save_name_on_exit(void* context) {
    SubBruteState* instance = (SubBruteState*)context;

    // Clear view
    void* validator_context = text_input_get_validator_callback_context(instance->text_input);
    text_input_set_validator(instance->text_input, NULL, NULL);
    validator_is_file_free(validator_context);

    text_input_reset(instance->text_input);

    furi_string_reset(instance->file_path);
}
