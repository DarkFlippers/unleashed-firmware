#include "../picopass_i.h"
#include "m-string.h"
#include <lib/toolbox/random_name.h>
#include <gui/modules/validators.h>
#include <toolbox/path.h>

void picopass_scene_save_name_text_input_callback(void* context) {
    Picopass* picopass = context;

    view_dispatcher_send_custom_event(picopass->view_dispatcher, PicopassCustomEventTextInputDone);
}

void picopass_scene_save_name_on_enter(void* context) {
    Picopass* picopass = context;

    // Setup view
    TextInput* text_input = picopass->text_input;
    bool dev_name_empty = false;
    if(!strcmp(picopass->dev->dev_name, "")) {
        set_random_name(picopass->text_store, sizeof(picopass->text_store));
        dev_name_empty = true;
    } else {
        picopass_text_store_set(picopass, picopass->dev->dev_name);
    }
    text_input_set_header_text(text_input, "Name the card");
    text_input_set_result_callback(
        text_input,
        picopass_scene_save_name_text_input_callback,
        picopass,
        picopass->text_store,
        PICOPASS_DEV_NAME_MAX_LEN,
        dev_name_empty);

    string_t folder_path;
    string_init(folder_path);

    if(string_end_with_str_p(picopass->dev->load_path, PICOPASS_APP_EXTENSION)) {
        path_extract_dirname(string_get_cstr(picopass->dev->load_path), folder_path);
    } else {
        string_set_str(folder_path, PICOPASS_APP_FOLDER);
    }

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        string_get_cstr(folder_path), PICOPASS_APP_EXTENSION, picopass->dev->dev_name);
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(picopass->view_dispatcher, PicopassViewTextInput);

    string_clear(folder_path);
}

bool picopass_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    Picopass* picopass = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PicopassCustomEventTextInputDone) {
            if(strcmp(picopass->dev->dev_name, "")) {
                // picopass_device_delete(picopass->dev, true);
            }
            strlcpy(
                picopass->dev->dev_name, picopass->text_store, strlen(picopass->text_store) + 1);
            if(picopass_device_save(picopass->dev, picopass->text_store)) {
                scene_manager_next_scene(picopass->scene_manager, PicopassSceneSaveSuccess);
                consumed = true;
            } else {
                consumed = scene_manager_search_and_switch_to_previous_scene(
                    picopass->scene_manager, PicopassSceneStart);
            }
        }
    }
    return consumed;
}

void picopass_scene_save_name_on_exit(void* context) {
    Picopass* picopass = context;

    // Clear view
    void* validator_context = text_input_get_validator_callback_context(picopass->text_input);
    text_input_set_validator(picopass->text_input, NULL, NULL);
    validator_is_file_free(validator_context);

    text_input_reset(picopass->text_input);
}
