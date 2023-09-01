#include "../lfrfid_i.h"
#include <dolphin/dolphin.h>
#include <toolbox/name_generator.h>

void lfrfid_scene_save_name_on_enter(void* context) {
    LfRfid* app = context;
    TextInput* text_input = app->text_input;
    FuriString* folder_path;
    folder_path = furi_string_alloc();

    bool key_name_is_empty = furi_string_empty(app->file_name);
    if(key_name_is_empty) {
        furi_string_set(app->file_path, LFRFID_APP_FOLDER);

        name_generator_make_auto(
            app->text_store, LFRFID_TEXT_STORE_SIZE, LFRFID_APP_FILENAME_PREFIX);

        furi_string_set(folder_path, LFRFID_APP_FOLDER);
    } else {
        lfrfid_text_store_set(app, "%s", furi_string_get_cstr(app->file_name));
        path_extract_dirname(furi_string_get_cstr(app->file_path), folder_path);
    }

    text_input_set_header_text(text_input, "Name the card");
    text_input_set_result_callback(
        text_input,
        lfrfid_text_input_callback,
        app,
        app->text_store,
        LFRFID_KEY_NAME_SIZE,
        key_name_is_empty);

    FURI_LOG_I("", "%s %s", furi_string_get_cstr(folder_path), app->text_store);

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        furi_string_get_cstr(folder_path),
        LFRFID_APP_FILENAME_EXTENSION,
        furi_string_get_cstr(app->file_name));
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    furi_string_free(folder_path);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewTextInput);
}

bool lfrfid_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventNext) {
            consumed = true;
            if(!furi_string_empty(app->file_name)) {
                lfrfid_delete_key(app);
            }

            furi_string_set(app->file_name, app->text_store);

            if(lfrfid_save_key(app)) {
                scene_manager_next_scene(scene_manager, LfRfidSceneSaveSuccess);
                if(scene_manager_has_previous_scene(scene_manager, LfRfidSceneSavedKeyMenu)) {
                    // Nothing, do not count editing as saving
                } else if(scene_manager_has_previous_scene(scene_manager, LfRfidSceneSaveType)) {
                    dolphin_deed(DolphinDeedRfidAdd);
                } else {
                    dolphin_deed(DolphinDeedRfidSave);
                }
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
