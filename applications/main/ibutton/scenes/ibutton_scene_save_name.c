#include "../ibutton_i.h"

#include <toolbox/random_name.h>
#include <toolbox/path.h>

#include <dolphin/dolphin.h>

static void ibutton_scene_save_name_text_input_callback(void* context) {
    iButton* ibutton = context;
    view_dispatcher_send_custom_event(ibutton->view_dispatcher, iButtonCustomEventTextEditResult);
}

void ibutton_scene_save_name_on_enter(void* context) {
    iButton* ibutton = context;
    TextInput* text_input = ibutton->text_input;

    const bool is_new_file = furi_string_empty(ibutton->file_path);

    if(is_new_file) {
        set_random_name(ibutton->key_name, IBUTTON_KEY_NAME_SIZE);
    }

    text_input_set_header_text(text_input, "Name the key");
    text_input_set_result_callback(
        text_input,
        ibutton_scene_save_name_text_input_callback,
        ibutton,
        ibutton->key_name,
        IBUTTON_KEY_NAME_SIZE,
        is_new_file);

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(IBUTTON_APP_FOLDER, IBUTTON_APP_EXTENSION, ibutton->key_name);
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(ibutton->view_dispatcher, iButtonViewTextInput);
}

bool ibutton_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    iButton* ibutton = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == iButtonCustomEventTextEditResult) {
            furi_string_printf(
                ibutton->file_path,
                "%s/%s%s",
                IBUTTON_APP_FOLDER,
                ibutton->key_name,
                IBUTTON_APP_EXTENSION);

            if(ibutton_save_key(ibutton)) {
                scene_manager_next_scene(ibutton->scene_manager, iButtonSceneSaveSuccess);

                if(scene_manager_has_previous_scene(
                       ibutton->scene_manager, iButtonSceneSavedKeyMenu)) {
                    // Nothing, do not count editing as saving
                } else if(scene_manager_has_previous_scene(
                              ibutton->scene_manager, iButtonSceneAddType)) {
                    DOLPHIN_DEED(DolphinDeedIbuttonAdd);
                } else {
                    DOLPHIN_DEED(DolphinDeedIbuttonSave);
                }

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
