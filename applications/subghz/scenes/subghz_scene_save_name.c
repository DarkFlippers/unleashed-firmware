#include "../subghz_i.h"
#include <lib/toolbox/random_name.h>
#include "file-worker.h"
#include "../helpers/subghz_custom_event.h"

void subghz_scene_save_name_text_input_callback(void* context) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, SubghzCustomEventSceneSaveName);
}

void subghz_scene_save_name_on_enter(void* context) {
    SubGhz* subghz = context;

    // Setup view
    TextInput* text_input = subghz->text_input;
    bool dev_name_empty = false;

    if(!strcmp(subghz->file_name, "")) {
        set_random_name(subghz->file_name, sizeof(subghz->file_name));
        dev_name_empty = true;
    } else {
        memcpy(subghz->file_name_tmp, subghz->file_name, strlen(subghz->file_name) + 1);
        if(scene_manager_get_scene_state(subghz->scene_manager, SubGhzSceneReadRAWMenu) ==
           SubghzCustomEventManagerSet) {
            subghz_get_next_name_file(subghz);
        }
    }

    text_input_set_header_text(text_input, "Name signal");
    text_input_set_result_callback(
        text_input,
        subghz_scene_save_name_text_input_callback,
        subghz,
        subghz->file_name,
        22, //Max len name
        dev_name_empty);
    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewTextInput);
}

bool subghz_scene_save_name_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == SubghzCustomEventSceneSaveName) {
            if(strcmp(subghz->file_name, "")) {
                if(strcmp(subghz->file_name_tmp, "")) {
                    subghz_rename_file(subghz);
                } else {
                    subghz_save_protocol_to_file(subghz, subghz->file_name);
                }

                subghz_file_name_clear(subghz);
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneSaveSuccess);
                return true;
            } else {
                string_set(subghz->error_str, "No name file");
                scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowError);
                return true;
            }
        }
    }
    return false;
}

void subghz_scene_save_name_on_exit(void* context) {
    SubGhz* subghz = context;

    // Clear view
    text_input_clean(subghz->text_input);
    scene_manager_set_scene_state(
        subghz->scene_manager, SubGhzSceneReadRAWMenu, SubghzCustomEventManagerNoSet);
}
