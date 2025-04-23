#include "../findmy_i.h"

enum ByteInputResult {
    ByteInputResultOk,
};

static void findmy_scene_config_packet_callback(void* context) {
    FindMy* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, ByteInputResultOk);
}

void findmy_scene_config_packet_on_enter(void* context) {
    FindMy* app = context;
    ByteInput* byte_input = app->byte_input;

    byte_input_set_header_text(byte_input, "Enter Bluetooth Payload:");

    memcpy(app->packet_buf, app->state.data, findmy_state_data_size(app->state.tag_type));

    byte_input_set_result_callback(
        byte_input,
        findmy_scene_config_packet_callback,
        NULL,
        app,
        app->packet_buf,
        findmy_state_data_size(app->state.tag_type));

    view_dispatcher_switch_to_view(app->view_dispatcher, FindMyViewByteInput);
}

bool findmy_scene_config_packet_on_event(void* context, SceneManagerEvent event) {
    FindMy* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        switch(event.event) {
        case ByteInputResultOk:
            scene_manager_search_and_switch_to_previous_scene(
                app->scene_manager, FindMySceneConfig);
            memcpy(app->state.data, app->packet_buf, findmy_state_data_size(app->state.tag_type));
            findmy_state_save_and_apply(&app->state);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void findmy_scene_config_packet_on_exit(void* context) {
    FindMy* app = context;

    byte_input_set_result_callback(app->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(app->byte_input, "");
}
