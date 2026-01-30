#include "../bad_duck3_app_i.h"

static void bad_duck3_scene_config_ble_mac_byte_input_callback(void* context) {
    BadDuck3App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

void bad_duck3_scene_config_ble_mac_on_enter(void* context) {
    BadDuck3App* app = context;

    memcpy(app->ble_mac_buf, app->script_hid_cfg.ble.mac, sizeof(app->ble_mac_buf));

    byte_input_set_header_text(app->byte_input, "Set BLE MAC Address");
    byte_input_set_result_callback(
        app->byte_input,
        bad_duck3_scene_config_ble_mac_byte_input_callback,
        NULL,
        app,
        app->ble_mac_buf,
        sizeof(app->ble_mac_buf));

    view_dispatcher_switch_to_view(app->view_dispatcher, BadDuck3AppViewByteInput);
}

bool bad_duck3_scene_config_ble_mac_on_event(void* context, SceneManagerEvent event) {
    BadDuck3App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const Duck3HidApi* hid = duck3_hid_get_interface(app->interface);
        memcpy(app->script_hid_cfg.ble.mac, app->ble_mac_buf, sizeof(app->script_hid_cfg.ble.mac));
        hid->adjust_config(&app->script_hid_cfg);
        memcpy(
            app->user_hid_cfg.ble.mac,
            app->script_hid_cfg.ble.mac,
            sizeof(app->user_hid_cfg.ble.mac));
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, BadDuck3SceneConfig);
        consumed = true;
    }

    return consumed;
}

void bad_duck3_scene_config_ble_mac_on_exit(void* context) {
    BadDuck3App* app = context;
    byte_input_set_result_callback(app->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(app->byte_input, "");
}
