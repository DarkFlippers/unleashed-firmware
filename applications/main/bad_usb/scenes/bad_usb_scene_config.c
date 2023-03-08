#include "../bad_usb_app_i.h"
#include "furi_hal_power.h"
#include "furi_hal_usb.h"

enum SubmenuIndex {
    SubmenuIndexKeyboardLayout,
};

void bad_usb_scene_config_submenu_callback(void* context, uint32_t index) {
    BadUsbApp* bad_usb = context;
    view_dispatcher_send_custom_event(bad_usb->view_dispatcher, index);
}

void bad_usb_scene_config_on_enter(void* context) {
    BadUsbApp* bad_usb = context;
    Submenu* submenu = bad_usb->submenu;

    submenu_add_item(
        submenu,
        "Keyboard Layout",
        SubmenuIndexKeyboardLayout,
        bad_usb_scene_config_submenu_callback,
        bad_usb);

    submenu_set_selected_item(
        submenu, scene_manager_get_scene_state(bad_usb->scene_manager, BadUsbSceneConfig));

    view_dispatcher_switch_to_view(bad_usb->view_dispatcher, BadUsbAppViewConfig);
}

bool bad_usb_scene_config_on_event(void* context, SceneManagerEvent event) {
    BadUsbApp* bad_usb = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(bad_usb->scene_manager, BadUsbSceneConfig, event.event);
        consumed = true;
        if(event.event == SubmenuIndexKeyboardLayout) {
            scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneConfigLayout);
        } else {
            furi_crash("Unknown key type");
        }
    }

    return consumed;
}

void bad_usb_scene_config_on_exit(void* context) {
    BadUsbApp* bad_usb = context;
    Submenu* submenu = bad_usb->submenu;

    submenu_reset(submenu);
}
