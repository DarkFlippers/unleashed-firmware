#include "../ibutton_i.h"
#include "m-string.h"
#include <lib/toolbox/random_name.h>
#include <toolbox/path.h>

static void ibutton_scene_save_name_text_input_callback(void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventTextEditResult);
}

void ibutton_scene_save_name_on_enter(void* context) {
    iButton* ibutton = context;
    TextInput* text_input = ibutton->text_input;

    string_t key_name;
    string_init(key_name);
    if(string_end_with_str_p(ibutton->file_path, IBUTTON_APP_EXTENSION)) {
        path_extract_filename(ibutton->file_path, key_name, true);
    }

    const bool key_name_is_empty = string_empty_p(key_name);
    if(key_name_is_empty) {
        set_random_name(ibutton->text_store, IBUTTON_TEXT_STORE_SIZE);
    } else {
        ibutton_text_store_set(ibutton, "%s", string_get_cstr(key_name));
    }

    text_input_set_header_text(text_input, "Name the key");
    text_input_set_result_callback(
        text_input,
        ibutton_scene_save_name_text_input_callback,
        ibutton,
        ibutton->text_store,
        IBUTTON_KEY_NAME_SIZE,
        key_name_is_empty);

    string_t folder_path;
    string_init(folder_path);

    path_extract_dirname(string_get_cstr(ibutton->file_path), folder_path);

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        string_get_cstr(folder_path), IBUTTON_APP_EXTENSION, string_get_cstr(key_name));
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewTextInput);

    string_clear(key_name);
    string_clear(folder_path);
}

bool ibutton_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventTextEditResult) {
            if(ibutton_save_key(ibutton, ibutton->text_store)) {
                scene_manager_next_scene(ibutton->scene_manager, iButtonSceneSaveSuccess);
            } else {
                const uint32_t possible_scenes[] = {
                    iButtonSceneReadKeyMenu, iButtonSceneSavedKeyMenu, iButtonSceneAddType};
                scene_manager_search_and_switch_to_previous_scene_one_of(
                    ibutton->scene_manager, possible_scenes, COUNT_OF(possible_scenes));
            }
        }
    }

    return consumed;
}

void ibutton_scene_save_name_on_exit(void* context) {
    iButton* ibutton = context;
    TextInput* text_input = ibutton->text_input;

    void* validator_context = text_input_get_validator_callback_context(text_input);
    text_input_set_validator(text_input, NULL, NULL);
    validator_is_file_free((ValidatorIsFile*)validator_context);

    text_input_reset(text_input);
}
