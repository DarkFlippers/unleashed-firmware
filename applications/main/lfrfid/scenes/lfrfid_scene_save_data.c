#include "../lfrfid_i.h"

void lfrfid_scene_save_data_on_enter(void* context) {
    LfRfid* app = context;
    ByteInput* byte_input = app->byte_input;

    size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);

    bool need_restore = scene_manager_get_scene_state(app->scene_manager, LfRfidSceneSaveData);

    if(!need_restore) {
        protocol_dict_get_data(app->dict, app->protocol_id, app->old_key_data, size);
        protocol_dict_get_data(app->dict, app->protocol_id, app->new_key_data, size);
    }

    byte_input_set_header_text(byte_input, "Enter the data in hex");

    byte_input_set_result_callback(
        byte_input, lfrfid_text_input_callback, NULL, app, app->new_key_data, size);

    view_dispatcher_switch_to_view(app->view_dispatcher, LfRfidViewByteInput);
}

bool lfrfid_scene_save_data_on_event(void* context, SceneManagerEvent event) {
    LfRfid* app = context;
    SceneManager* scene_manager = app->scene_manager;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == LfRfidEventNext) {
            consumed = true;
            size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
            protocol_dict_set_data(app->dict, app->protocol_id, app->new_key_data, size);

            if(scene_manager_has_previous_scene(scene_manager, LfRfidSceneSaveType)) {
                scene_manager_next_scene(scene_manager, LfRfidSceneSaveName);
            } else {
                if(!furi_string_empty(app->file_name)) {
                    lfrfid_delete_key(app);
                }

                if(lfrfid_save_key(app)) {
                    scene_manager_next_scene(scene_manager, LfRfidSceneSaveSuccess);
                } else {
                    scene_manager_search_and_switch_to_previous_scene(
                        scene_manager, LfRfidSceneSavedKeyMenu);
                }
            }
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_set_scene_state(scene_manager, LfRfidSceneSaveData, 0);
        size_t size = protocol_dict_get_data_size(app->dict, app->protocol_id);
        protocol_dict_set_data(app->dict, app->protocol_id, app->old_key_data, size);
    }

    return consumed;
}

void lfrfid_scene_save_data_on_exit(void* context) {
    UNUSED(context);
}
