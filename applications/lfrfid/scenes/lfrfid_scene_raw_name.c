#include "../lfrfid_i.h"

void lfrfid_scene_raw_name_on_enter(void* context) {
    LfRfid* app = context;
    TextInput* text_input = app->text_input;

    const char* key_name = string_get_cstr(app->raw_file_name);

    bool key_name_is_empty = string_empty_p(app->file_name);
    if(key_name_is_empty) {
        lfrfid_text_store_set(app, "RfidRecord");
    } else {
        lfrfid_text_store_set(app, "%s", key_name);
    }

    text_input_set_header_text(text_input, "Name the raw file");

    text_input_set_result_callback(
        text_input,
        lfrfid_text_input_callback,
        app,
        app->text_store,
        LFRFID_KEY_NAME_SIZE,
        key_name_is_empty);

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(LFRFID_SD_FOLDER, LFRFID_APP_RAW_ASK_EXTENSION, NULL);
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewTextInput);
}

bool lfrfid_scene_raw_name_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventNext) {
            consumed = true;
            string_set_str(app->raw_file_name, app->text_store);
            scene_manager_next_scene(scene_manager, LfRfidSceneRawInfo);
        }
    }

    return consumed;
}

void lfrfid_scene_raw_name_on_exit(void* context) {
    LfRfid* app = context;
    TextInput* text_input = app->text_input;

    void* validator_context = text_input_get_validator_callback_context(text_input);
    text_input_set_validator(text_input, NULL, NULL);
    validator_is_file_free((ValidatorIsFile*)validator_context);

    text_input_reset(text_input);
}
