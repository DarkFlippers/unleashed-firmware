#include "../bad_duck3_app_i.h"

static void bad_duck3_scene_config_ble_name_text_input_callback(void* context) {
    BadDuck3App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

void bad_duck3_scene_config_ble_name_on_enter(void* context) {
    BadDuck3App* app = context;

    strlcpy(app->ble_name_buf, app->script_hid_cfg.ble.name, sizeof(app->ble_name_buf));

    text_input_set_header_text(app->text_input, "Set BLE Device Name");
    text_input_set_result_callback(
        app->text_input,
        bad_duck3_scene_config_ble_name_text_input_callback,
        app,
        app->ble_name_buf,
        sizeof(app->ble_name_buf),
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, BadDuck3AppViewTextInput);
}

bool bad_duck3_scene_config_ble_name_on_event(void* context, SceneManagerEvent event) {
    BadDuck3App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const Duck3HidApi* hid = duck3_hid_get_interface(app->interface);
        strlcpy(
            app->script_hid_cfg.ble.name, app->ble_name_buf, sizeof(app->script_hid_cfg.ble.name));
        hid->adjust_config(&app->script_hid_cfg);
        strlcpy(
            app->user_hid_cfg.ble.name,
            app->script_hid_cfg.ble.name,
            sizeof(app->user_hid_cfg.ble.name));
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, BadDuck3SceneConfig);
        consumed = true;
    }

    return consumed;
}

void bad_duck3_scene_config_ble_name_on_exit(void* context) {
    BadDuck3App* app = context;
    text_input_reset(app->text_input);
}
