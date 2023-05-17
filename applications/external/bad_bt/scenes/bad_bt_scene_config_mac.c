#include "../bad_bt_app.h"

#define TAG "BadBtConfigMac"

void bad_bt_scene_config_mac_byte_input_callback(void* context) {
    BadBtApp* bad_bt = context;

    view_dispatcher_send_custom_event(bad_bt->view_dispatcher, BadBtAppCustomEventByteInputDone);
}

void bad_bt_scene_config_mac_on_enter(void* context) {
    BadBtApp* bad_bt = context;

    // Setup view
    ByteInput* byte_input = bad_bt->byte_input;
    byte_input_set_header_text(byte_input, "Set BT MAC address");
    byte_input_set_result_callback(
        byte_input,
        bad_bt_scene_config_mac_byte_input_callback,
        NULL,
        bad_bt,
        bad_bt->config.bt_mac,
        GAP_MAC_ADDR_SIZE);
    view_dispatcher_switch_to_view(bad_bt->view_dispatcher, BadBtAppViewConfigMac);
}

bool bad_bt_scene_config_mac_on_event(void* context, SceneManagerEvent event) {
    BadBtApp* bad_bt = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BadBtAppCustomEventByteInputDone) {
            bt_set_profile_mac_address(bad_bt->bt, bad_bt->config.bt_mac);
            scene_manager_previous_scene(bad_bt->scene_manager);
            consumed = true;
        }
    }
    return consumed;
}

void bad_bt_scene_config_mac_on_exit(void* context) {
    BadBtApp* bad_bt = context;

    // Clear view
    byte_input_set_result_callback(bad_bt->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(bad_bt->byte_input, "");
}
