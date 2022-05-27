#include "../subghz_i.h"
#include <lib/toolbox/random_name.h>
#include "../helpers/subghz_custom_event.h"
#include <lib/subghz/protocols/raw.h>
#include <gui/modules/validators.h>

#define MAX_TEXT_INPUT_LEN 22

void subghz_scene_save_name_text_input_callback(void* context) {
    furi_assert(context);
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubGhzCustomEventSceneSaveName);
}

void subghz_scene_save_name_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    TextInput* text_input = subghz->text_input;
    bool dev_name_empty = false;

    string_t file_name;
    string_init(file_name);

    if(!strcmp(subghz->file_path, "")) {
        char file_name_buf[SUBGHZ_MAX_LEN_NAME] = {0};
        set_random_name(file_name_buf, SUBGHZ_MAX_LEN_NAME);
        string_set(file_name, file_name_buf);
        strncpy(subghz->file_dir, SUBGHZ_APP_FOLDER, SUBGHZ_MAX_LEN_NAME);
        //highlighting the entire filename by default
        dev_name_empty = true;
    } else {
        strncpy(subghz->file_path_tmp, subghz->file_path, SUBGHZ_MAX_LEN_NAME);
        path_extract_dirname(subghz->file_path, file_name);
        strncpy(subghz->file_dir, string_get_cstr(file_name), SUBGHZ_MAX_LEN_NAME);
        path_extract_filename_no_ext(subghz->file_path, file_name);
        if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneReadRAW) !=
           SubGhzCustomEventManagerNoSet) {
            subghz_get_next_name_file(subghz, SUBGHZ_MAX_LEN_NAME);
            path_extract_filename_no_ext(subghz->file_path, file_name);
            if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneReadRAW) ==
               SubGhzCustomEventManagerSetRAW) {
                dev_name_empty = true;
            }
        }
    }
    strncpy(subghz->file_path, string_get_cstr(file_name), SUBGHZ_MAX_LEN_NAME);
    text_input_set_header_text(text_input, "Name signal");
    text_input_set_result_callback(
        text_input,
        subghz_scene_save_name_text_input_callback,
        subghz,
        subghz->file_path,
        MAX_TEXT_INPUT_LEN, // buffer size
        dev_name_empty);

    ValidatorIsFile* validator_is_file =
        validator_is_file_alloc_init(subghz->file_dir, SUBGHZ_APP_EXTENSION, NULL);
    text_input_set_validator(text_input, validator_is_file_callback, validator_is_file);

    string_clear(file_name);

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdTextInput);
}

bool subghz_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    if(event.type == SceneManagerEventTypeBack) {
        strncpy(subghz->file_path, subghz->file_path_tmp, SUBGHZ_MAX_LEN_NAME);
        scene_manager_previous_scene(subghz->scene_manager);
        return true;
    } else if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubGhzCustomEventSceneSaveName) {
            if(strcmp(subghz->file_path, "")) {
                string_t temp_str;
                string_init_printf(
                    temp_str, "%s/%s%s", subghz->file_dir, subghz->file_path, SUBGHZ_APP_EXTENSION);
                strncpy(subghz->file_path, string_get_cstr(temp_str), SUBGHZ_MAX_LEN_NAME);
                string_clear(temp_str);
                if(strcmp(subghz->file_path_tmp, "")) {
                    if(!subghz_rename_file(subghz)) {
                        return false;
                    }
                } else {
                    if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneSetType) !=
                       SubGhzCustomEventManagerNoSet) {
                        subghz_save_protocol_to_file(
                            subghz, subghz->txrx->fff_data, subghz->file_path);
                        scene_manager_set_scene_state(
                            subghz->scene_manager,
                            SubGhzSceneSetType,
                            SubGhzCustomEventManagerNoSet);
                    } else {
                        subghz_save_protocol_to_file(
                            subghz,
                            subghz_history_get_raw_data(
                                subghz->txrx->history, subghz->txrx->idx_menu_chosen),
                            subghz->file_path);
                    }
                }

                if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneReadRAW) !=
                   SubGhzCustomEventManagerNoSet) {
                    subghz_protocol_raw_gen_fff_data(subghz->txrx->fff_data, subghz->file_path);
                    scene_manager_set_scene_state(
                        subghz->scene_manager, SubGhzSceneReadRAW, SubGhzCustomEventManagerNoSet);
                } else {
                    subghz_file_name_clear(subghz);
                }

                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveSuccess);
                return true;
            } else {
                string_set(subghz->error_str, "No name file");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
                return true;
            }
        }
    }
    return false;
}

void subghz_scene_save_name_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear validator
    void* validator_context = text_input_get_validator_callback_context(subghz->text_input);
    text_input_set_validator(subghz->text_input, NULL, NULL);
    validator_is_file_free(validator_context);

    // Clear view
    text_input_reset(subghz->text_input);
}
