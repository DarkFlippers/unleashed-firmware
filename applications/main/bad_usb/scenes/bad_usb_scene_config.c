#include "../bad_usb_app_i.h"

enum SubmenuIndex {
    ConfigIndexKeyboardLayout,
    ConfigIndexBleUnpair,
};

void bad_usb_scene_config_select_callback(void* context, uint32_t index) {
    BadUsbApp* bad_usb = context;

    view_dispatcher_send_custom_event(bad_usb->view_dispatcher, index);
}

static void draw_menu(BadUsbApp* bad_usb) {
    VariableItemList* var_item_list = bad_usb->var_item_list;

    variable_item_list_reset(var_item_list);

    variable_item_list_add(var_item_list, "Keyboard Layout (global)", 0, NULL, NULL);

    variable_item_list_add(var_item_list, "Remove Pairing", 0, NULL, NULL);
}

void bad_usb_scene_config_on_enter(void* context) {
    BadUsbApp* bad_usb = context;
    VariableItemList* var_item_list = bad_usb->var_item_list;

    variable_item_list_set_enter_callback(
        var_item_list, bad_usb_scene_config_select_callback, bad_usb);
    draw_menu(bad_usb);
    variable_item_list_set_selected_item(var_item_list, 0);

    view_dispatcher_switch_to_view(bad_usb->view_dispatcher, BadUsbAppViewConfig);
}

bool bad_usb_scene_config_on_event(void* context, SceneManagerEvent event) {
    BadUsbApp* bad_usb = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == ConfigIndexKeyboardLayout) {
            scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneConfigLayout);
        } else if(event.event == ConfigIndexBleUnpair) {
            scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneConfirmUnpair);
        } else {
            furi_crash("Unknown key type");
        }
    }

    return consumed;
}

void bad_usb_scene_config_on_exit(void* context) {
    BadUsbApp* bad_usb = context;
    VariableItemList* var_item_list = bad_usb->var_item_list;

    variable_item_list_reset(var_item_list);
}
