#include "../subghz_i.h"

bool subghz_scene_saved_file_select(SubGhz* subghz) {
    furi_assert(subghz);

    FileWorker* file_worker = file_worker_alloc(false);
    string_t protocol_file_name;
    string_init(protocol_file_name);
    string_t temp_str;
    string_init(temp_str);

    // Input events and views are managed by file_select
    bool res = file_worker_file_select(
        file_worker,
        SUBGHZ_APP_PATH_FOLDER,
        SUBGHZ_APP_EXTENSION,
        subghz->text_store,
        sizeof(subghz->text_store),
        NULL);

    if(res) {
        // Get key file path
        string_printf(
            protocol_file_name,
            "%s/%s%s",
            SUBGHZ_APP_PATH_FOLDER,
            subghz->text_store,
            SUBGHZ_APP_EXTENSION);
    } else {
        string_clear(temp_str);
        string_clear(protocol_file_name);

        file_worker_close(file_worker);
        file_worker_free(file_worker);
        return res;
    }

    do {
        if(!file_worker_open(
               file_worker, string_get_cstr(protocol_file_name), FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }
        // Read and parse name protocol from 1st line
        if(!file_worker_read_until(file_worker, temp_str, '\n')) {
            break;
        }
        // strlen("Protocol: ") = 10
        string_right(temp_str, 10);
        subghz->protocol_result =
            subghz_protocol_get_by_name(subghz->protocol, string_get_cstr(temp_str));
        if(subghz->protocol_result == NULL) {
            file_worker_show_error(file_worker, "Cannot parse\nfile");
            break;
        }
        if(!subghz->protocol_result->to_load_protocol(file_worker, subghz->protocol_result)) {
            file_worker_show_error(file_worker, "Cannot parse\nfile");
            break;
        }
        res = true;
    } while(0);

    string_clear(temp_str);
    string_clear(protocol_file_name);

    file_worker_close(file_worker);
    file_worker_free(file_worker);

    return res;
}

const void subghz_scene_saved_on_enter(void* context) {
    SubGhz* subghz = context;

    if(subghz_scene_saved_file_select(subghz)) {
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneTransmitter);
    } else {
        scene_manager_search_and_switch_to_previous_scene(subghz->scene_manager, SubGhzSceneStart);
    }
}

const bool subghz_scene_saved_on_event(void* context, SceneManagerEvent event) {
    // SubGhz* subghz = context;
    return false;
}

const void subghz_scene_saved_on_exit(void* context) {
    // SubGhz* subghz = context;
}
