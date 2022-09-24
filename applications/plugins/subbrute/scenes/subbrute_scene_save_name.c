#include <m-string.h>
#include <subghz/types.h>
#include <lib/toolbox/random_name.h>
#include <gui/modules/validators.h>
#include <lib/toolbox/path.h>

#include "../subbrute_i.h"
#include "../subbrute_custom_event.h"
#include "../views/subbrute_attack_view.h"

#define TAG "SubBruteSceneSaveFile"

void subbrute_scene_save_name_on_enter(void* context) {
    SubBruteState* instance = (SubBruteState*)context;

    // Setup view
    TextInput* text_input = instance->text_input;
    set_random_name(instance->device->text_store, sizeof(instance->device->text_store));

    text_input_set_header_text(text_input, "Name of file");
    text_input_set_result_callback(
        text_input,
        subbrute_text_input_callback,
        instance,
        instance->device->text_store,
        SUBBRUTE_MAX_LEN_NAME,
        true);

    string_t folder_path;
    string_init(folder_path);

    SubBruteDevice* device = instance->device;
    if(string_end_with_str_p(device->load_path, SUBBRUTE_FILE_EXT)) {
        path_extract_dirname(string_get_cstr(device->load_path), folder_path);
    } else {
        string_set_str(folder_path, SUBBRUTE_PATH);
    }

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(string_get_cstr(folder_path), SUBBRUTE_FILE_EXT, TAG);
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(instance->view_dispatcher, SubBruteViewTextInput);

    string_clear(folder_path);
}

bool subbrute_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    SubBruteState* instance = (SubBruteState*)context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom &&
       event.event == SubBruteCustomEventTypeTextEditDone) {
        if(subbrute_device_save_file(instance->device, instance->device->text_store)) {
            scene_manager_next_scene(instance->scene_manager, SubBruteSceneSaveSuccess);
            consumed = true;
        } else {
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
}
