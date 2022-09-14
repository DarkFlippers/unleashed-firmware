#include "m-string.h"
#include <lib/toolbox/random_name.h>
#include "../lfrfid_i.h"

void lfrfid_scene_save_name_on_enter(void* context) {
    LfRfid* app = context;
    TextInput* text_input = app->text_input;
    string_t folder_path;
    string_init(folder_path);

    bool key_name_is_empty = string_empty_p(app->file_name);
    if(key_name_is_empty) {
        string_set_str(app->file_path, LFRFID_APP_FOLDER);
        set_random_name(app->text_store, LFRFID_TEXT_STORE_SIZE);
        string_set_str(folder_path, LFRFID_APP_FOLDER);
    } else {
        lfrfid_text_store_set(app, "%s", string_get_cstr(app->file_name));
        path_extract_dirname(string_get_cstr(app->file_path), folder_path);
    }

    text_input_set_header_text(text_input, "Name the card");
    text_input_set_result_callback(
        text_input,
        lfrfid_text_input_callback,
        app,
        app->text_store,
        LFRFID_KEY_NAME_SIZE,
        key_name_is_empty);

    FURI_LOG_I("", "%s %s", string_get_cstr(folder_path), app->text_store);

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        string_get_cstr(folder_path), LFRFID_APP_EXTENSION, string_get_cstr(app->file_name));
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    string_clear(folder_path);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewTextInput);
}

bool lfrfid_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventNext) {
            consumed = true;
            if(!string_empty_p(app->file_name)) {
                lfrfid_delete_key(app);
            }

            string_set_str(app->file_name, app->text_store);

            if(lfrfid_save_key(app)) {
                scene_manager_next_scene(scene_manager, LfRfidSceneSaveSuccess);
            } else {
                scene_manager_search_and_switch_to_previous_scene(
                    scene_manager, LfRfidSceneReadKeyMenu);
            }
        }
    }

    return consumed;
}

void lfrfid_scene_save_name_on_exit(void* context) {
    LfRfid* app = context;
    TextInput* text_input = app->text_input;

    void* validator_context = text_input_get_validator_callback_context(text_input);
    text_input_set_validator(text_input, NULL, NULL);
    validator_is_file_free((ValidatorIsFile*)validator_context);

    text_input_reset(text_input);
}
