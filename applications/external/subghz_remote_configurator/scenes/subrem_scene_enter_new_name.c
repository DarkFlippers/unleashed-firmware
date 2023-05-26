#include "../subghz_remote_app_i.h"
#include "../helpers/subrem_custom_event.h"

#include <gui/modules/validators.h>

void subrem_scene_enter_new_name_text_input_callback(void* context) {
    furi_assert(context);
    SubGhzRemoteApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, SubRemCustomEventSceneNewName);
}

void subrem_scene_enter_new_name_on_enter(void* context) {
    SubGhzRemoteApp* app = context;

    // Setup view
    TextInput* text_input = app->text_input;

    //strncpy(app->file_name_tmp, "subrem_", SUBREM_MAX_LEN_NAME);
    text_input_set_header_text(text_input, "Map file Name");
    text_input_set_result_callback(
        text_input,
        subrem_scene_enter_new_name_text_input_callback,
        app,
        app->file_name_tmp,
        25,
        false);

    ValidatorIsFile* validator_is_file = validator_is_file_alloc_init(
        furi_string_get_cstr(app->file_path), SUBREM_APP_EXTENSION, "");
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    view_dispatcher_switch_to_view(app->view_dispatcher, SubRemViewIDTextInput);
}

bool subrem_scene_enter_new_name_on_event(void* context, SceneManagerEvent event) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubRemCustomEventSceneNewName) {
            if(strcmp(app->file_name_tmp, "") != 0) {
                furi_string_set(app->file_path, SUBREM_APP_FOLDER);
                furi_string_cat_printf(
                    app->file_path, "/%s%s", app->file_name_tmp, SUBREM_APP_EXTENSION);

                subrem_map_preset_reset(app->map_preset);
                scene_manager_next_scene(app->scene_manager, SubRemSceneEditMenu);
            } else { //error
            }
            consumed = true;
        }
    }

    return consumed;
}

void subrem_scene_enter_new_name_on_exit(void* context) {
    furi_assert(context);

    SubGhzRemoteApp* app = context;
    submenu_reset(app->submenu);

    // Clear validator & view
    void* validator_context = text_input_get_validator_callback_context(app->text_input);
    text_input_set_validator(app->text_input, NULL, NULL);
    validator_is_file_free(validator_context);
    text_input_reset(app->text_input);
}
