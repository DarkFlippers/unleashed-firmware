#include "../ibutton_i.h"
#include <lib/toolbox/random_name.h>

static void ibutton_scene_save_name_text_input_callback(void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventTextEditResult);
}

void ibutton_scene_save_name_on_enter(void* context) {
    iButton* ibutton = context;
    TextInput* text_input = ibutton->text_input;

    const char* key_name = ibutton_key_get_name_p(ibutton->key);
    const bool key_name_is_empty = !strcmp(key_name, "");

    if(key_name_is_empty) {
        set_random_name(ibutton->text_store, IBUTTON_TEXT_STORE_SIZE);
    } else {
        ibutton_text_store_set(ibutton, "%s", key_name);
    }

    text_input_set_header_text(text_input, "Name the key");
    text_input_set_result_callback(
        text_input,
        ibutton_scene_save_name_text_input_callback,
        ibutton,
        ibutton->text_store,
        IBUTTON_KEY_NAME_SIZE,
        key_name_is_empty);

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(IBUTTON_APP_FOLDER, IBUTTON_APP_EXTENSION, key_name);
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewTextInput);
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
                ibutton_switch_to_previous_scene_one_of(
                    ibutton, possible_scenes, sizeof(possible_scenes) / sizeof(uint32_t));
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
