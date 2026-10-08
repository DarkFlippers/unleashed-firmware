#include "../bad_usb_app_i.h"

enum TextInputResult {
    TextInputResultOk,
};

static void bad_usb_scene_config_usb_name_text_input_callback(void* context) {
    BadUsbApp* bad_usb = context;

    view_dispatcher_send_custom_event(bad_usb->view_dispatcher, TextInputResultOk);
}

void bad_usb_scene_config_usb_name_on_enter(void* context) {
    BadUsbApp* bad_usb = context;
    TextInput* text_input = bad_usb->text_input;

    if(scene_manager_get_scene_state(bad_usb->scene_manager, BadUsbSceneConfigUsbName)) {
        strlcpy(
            bad_usb->usb_name_buf,
            bad_usb->script_hid_cfg.usb.manuf,
            sizeof(bad_usb->usb_name_buf));
        text_input_set_header_text(text_input, "Set USB manufacturer name");
    } else {
        strlcpy(
            bad_usb->usb_name_buf,
            bad_usb->script_hid_cfg.usb.product,
            sizeof(bad_usb->usb_name_buf));
        text_input_set_header_text(text_input, "Set USB product name");
    }

    text_input_set_result_callback(
        text_input,
        bad_usb_scene_config_usb_name_text_input_callback,
        bad_usb,
        bad_usb->usb_name_buf,
        sizeof(bad_usb->usb_name_buf),
        true);

    view_dispatcher_switch_to_view(bad_usb->view_dispatcher, BadUsbAppViewTextInput);
}

bool bad_usb_scene_config_usb_name_on_event(void* context, SceneManagerEvent event) {
    BadUsbApp* bad_usb = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == TextInputResultOk) {
            const BadUsbHidApi* hid = bad_usb_hid_get_interface(bad_usb->interface);
            if(scene_manager_get_scene_state(bad_usb->scene_manager, BadUsbSceneConfigUsbName)) {
                // Apply to current script config
                strlcpy(
                    bad_usb->script_hid_cfg.usb.manuf,
                    bad_usb->usb_name_buf,
                    sizeof(bad_usb->script_hid_cfg.usb.manuf));
                hid->adjust_config(&bad_usb->script_hid_cfg);
                // Set in user config to save in settings file
                strlcpy(
                    bad_usb->user_hid_cfg.usb.manuf,
                    bad_usb->usb_name_buf,
                    sizeof(bad_usb->user_hid_cfg.usb.manuf));
            } else {
                // Apply to current script config
                strlcpy(
                    bad_usb->script_hid_cfg.usb.product,
                    bad_usb->usb_name_buf,
                    sizeof(bad_usb->script_hid_cfg.usb.product));
                hid->adjust_config(&bad_usb->script_hid_cfg);
                // Set in user config to save in settings file
                strlcpy(
                    bad_usb->user_hid_cfg.usb.product,
                    bad_usb->usb_name_buf,
                    sizeof(bad_usb->user_hid_cfg.usb.product));
            }
        }
        scene_manager_previous_scene(bad_usb->scene_manager);
    }
    return consumed;
}

void bad_usb_scene_config_usb_name_on_exit(void* context) {
    BadUsbApp* bad_usb = context;
    TextInput* text_input = bad_usb->text_input;

    text_input_reset(text_input);
}
