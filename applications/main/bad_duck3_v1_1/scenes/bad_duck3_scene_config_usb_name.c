#include "../bad_duck3_app_i.h"

static void bad_duck3_scene_config_usb_name_text_input_callback(void* context) {
    BadDuck3App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, 0);
}

void bad_duck3_scene_config_usb_name_on_enter(void* context) {
    BadDuck3App* app = context;
    bool is_manuf = scene_manager_get_scene_state(app->scene_manager, BadDuck3SceneConfigUsbName);

    if(is_manuf) {
        strlcpy(app->usb_name_buf, app->script_hid_cfg.usb.manuf, sizeof(app->usb_name_buf));
        text_input_set_header_text(app->text_input, "Set Manufacturer Name");
    } else {
        strlcpy(app->usb_name_buf, app->script_hid_cfg.usb.product, sizeof(app->usb_name_buf));
        text_input_set_header_text(app->text_input, "Set Product Name");
    }

    text_input_set_result_callback(
        app->text_input,
        bad_duck3_scene_config_usb_name_text_input_callback,
        app,
        app->usb_name_buf,
        sizeof(app->usb_name_buf),
        false);

    view_dispatcher_switch_to_view(app->view_dispatcher, BadDuck3AppViewTextInput);
}

bool bad_duck3_scene_config_usb_name_on_event(void* context, SceneManagerEvent event) {
    BadDuck3App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        bool is_manuf =
            scene_manager_get_scene_state(app->scene_manager, BadDuck3SceneConfigUsbName);
        const Duck3HidApi* hid = duck3_hid_get_interface(app->interface);

        if(is_manuf) {
            strlcpy(
                app->script_hid_cfg.usb.manuf,
                app->usb_name_buf,
                sizeof(app->script_hid_cfg.usb.manuf));
            strlcpy(
                app->user_hid_cfg.usb.manuf,
                app->script_hid_cfg.usb.manuf,
                sizeof(app->user_hid_cfg.usb.manuf));
        } else {
            strlcpy(
                app->script_hid_cfg.usb.product,
                app->usb_name_buf,
                sizeof(app->script_hid_cfg.usb.product));
            strlcpy(
                app->user_hid_cfg.usb.product,
                app->script_hid_cfg.usb.product,
                sizeof(app->user_hid_cfg.usb.product));
        }
        hid->adjust_config(&app->script_hid_cfg);
        scene_manager_search_and_switch_to_previous_scene(
            app->scene_manager, BadDuck3SceneConfig);
        consumed = true;
    }

    return consumed;
}

void bad_duck3_scene_config_usb_name_on_exit(void* context) {
    BadDuck3App* app = context;
    text_input_reset(app->text_input);
}
