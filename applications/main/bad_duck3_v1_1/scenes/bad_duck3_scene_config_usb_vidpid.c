#include "../bad_duck3_app_i.h"

static void bad_duck3_scene_config_usb_vidpid_byte_input_callback(void* context) {
    BadDuck3App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

void bad_duck3_scene_config_usb_vidpid_on_enter(void* context) {
    BadDuck3App* app = context;

    app->usb_vidpid_buf[0] = app->script_hid_cfg.usb.vid;
    app->usb_vidpid_buf[1] = app->script_hid_cfg.usb.pid;

    byte_input_set_header_text(app->byte_input, "Set VID:PID (hex)");
    byte_input_set_result_callback(
        app->byte_input,
        bad_duck3_scene_config_usb_vidpid_byte_input_callback,
        NULL,
        app,
        (uint8_t*)app->usb_vidpid_buf,
        sizeof(app->usb_vidpid_buf));

    view_dispatcher_switch_to_view(app->view_dispatcher, BadDuck3AppViewByteInput);
}

bool bad_duck3_scene_config_usb_vidpid_on_event(void* context, SceneManagerEvent event) {
    BadDuck3App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        const Duck3HidApi* hid = duck3_hid_get_interface(app->interface);
        app->script_hid_cfg.usb.vid = app->usb_vidpid_buf[0];
        app->script_hid_cfg.usb.pid = app->usb_vidpid_buf[1];
        hid->adjust_config(&app->script_hid_cfg);
        app->user_hid_cfg.usb.vid = app->script_hid_cfg.usb.vid;
        app->user_hid_cfg.usb.pid = app->script_hid_cfg.usb.pid;
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, BadDuck3SceneConfig);
        consumed = true;
    }

    return consumed;
}

void bad_duck3_scene_config_usb_vidpid_on_exit(void* context) {
    BadDuck3App* app = context;
    byte_input_set_result_callback(app->byte_input, NULL, NULL, NULL, NULL, 0);
    byte_input_set_header_text(app->byte_input, "");
}
