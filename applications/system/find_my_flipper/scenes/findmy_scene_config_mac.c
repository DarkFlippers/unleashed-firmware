#include "../findmy_i.h"

enum ByteInputResult {
    ByteInputResultOk,
};

static void findmy_scene_config_mac_callback(void* context) {
    FindMy* app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, ByteInputResultOk);
}

void findmy_scene_config_mac_on_enter(void* context) {
    FindMy* app = context;
    ByteInput* byte_input = app->byte_input;

    byte_input_set_header_text(byte_input, "Enter Bluetooth MAC:");

    memcpy(app->mac_buf, app->state.mac, sizeof(app->mac_buf));
    findmy_reverse_mac_addr(app->mac_buf);

    byte_input_set_result_callback(
        byte_input,
        findmy_scene_config_mac_callback,
        NULL,
        app,
        app->mac_buf,
        sizeof(app->mac_buf));

    view_dispatcher_switch_to_view(app->view_dispatcher, FindMyViewByteInput);
}

bool findmy_scene_config_mac_on_event(void* context, SceneManagerEvent event) {
    FindMy* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        switch(event.event) {
        case ByteInputResultOk:
            findmy_reverse_mac_addr(app->mac_buf);
            memcpy(&app->state.mac, app->mac_buf, sizeof(app->state.mac));
            findmy_state_save_and_apply(&app->state);
            findmy_main_update_mac(app->findmy_main, app->state.mac);
            scene_manager_next_scene(app->scene_manager, FindMySceneConfigPacket);
            break;
        default:
            break;
        }
    }

    return consumed;
}

void findmy_scene_config_mac_on_exit(void* context) {
    FindMy* app = context;

    byte_input_set_result_callback(app->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(app->byte_input, "");
}
