#include "../subghz_i.h"
#include <lib/toolbox/random_name.h>
#include "file-worker.h"

#define SCENE_SAVE_NAME_CUSTOM_EVENT (0UL)

bool subghz_scene_save_data_to_file(void* context, const char* dev_name) {
    SubGhz* subghz = context;
    FileWorker* file_worker = file_worker_alloc(false);
    string_t dev_file_name;
    string_init(dev_file_name);
    string_t temp_str;
    string_init(temp_str);
    bool saved = false;

    do {
        // Create subghz folder directory if necessary
        if(!file_worker_mkdir(file_worker, SUBGHZ_APP_FOLDER)) {
            break;
        }
        // Create saved directory if necessary
        if(!file_worker_mkdir(file_worker, SUBGHZ_APP_PATH_FOLDER)) {
            break;
        }
        // First remove subghz device file if it was saved
        string_printf(
            dev_file_name, "%s/%s%s", SUBGHZ_APP_PATH_FOLDER, dev_name, SUBGHZ_APP_EXTENSION);
        if(!file_worker_remove(file_worker, string_get_cstr(dev_file_name))) {
            break;
        }
        // Open file
        if(!file_worker_open(
               file_worker, string_get_cstr(dev_file_name), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            break;
        }
        //Get string save
        subghz->protocol_result->to_save_string(subghz->protocol_result, temp_str);
        // Prepare and write data to file
        if(!file_worker_write(file_worker, string_get_cstr(temp_str), string_size(temp_str))) {
            break;
        }
        saved = true;
    } while(0);

    string_clear(temp_str);
    string_clear(dev_file_name);
    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return saved;
}

void subghz_scene_save_name_text_input_callback(void* context) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SCENE_SAVE_NAME_CUSTOM_EVENT);
}

const void subghz_scene_save_name_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    TextInput* text_input = subghz->text_input;
    bool dev_name_empty = false;

    set_random_name(subghz->text_store, sizeof(subghz->text_store));
    dev_name_empty = true;

    text_input_set_header_text(text_input, "Name the KEY");
    text_input_set_result_callback(
        text_input,
        subghz_scene_save_name_text_input_callback,
        subghz,
        subghz->text_store,
        22, //Max len name
        dev_name_empty);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewTextInput);
}

const bool subghz_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SCENE_SAVE_NAME_CUSTOM_EVENT) {
            if(subghz_scene_save_data_to_file(subghz, subghz->text_store)) {
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveSuccess);
                return true;
            } else {
                //Error save
                return true;
            }
        }
    }
    return false;
}

const void subghz_scene_save_name_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear view
    text_input_set_header_text(subghz->text_input, NULL);
    text_input_set_result_callback(subghz->text_input, NULL, NULL, NULL, 0, false);
}
