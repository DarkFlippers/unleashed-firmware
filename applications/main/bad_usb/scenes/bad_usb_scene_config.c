#include "../bad_usb_app_i.h"

enum SubmenuIndex {
    ConfigIndexKeyboardLayout,
    ConfigIndexInterface,
    ConfigIndexBleUnpair,
};

const char* const interface_mode_text[2] = {
    "USB",
    "BLE",
};

void bad_usb_scene_config_select_callback(void* context, uint32_t index) {
    BadUsbApp* bad_usb = context;
    if(index != ConfigIndexInterface) {
        view_dispatcher_send_custom_event(bad_usb->view_dispatcher, index);
    }
}

void bad_usb_scene_config_interface_callback(VariableItem* item) {
    BadUsbApp* bad_usb = variable_item_get_context(item);
    furi_assert(bad_usb);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, interface_mode_text[index]);
    bad_usb->interface = index;

    view_dispatcher_send_custom_event(bad_usb->view_dispatcher, ConfigIndexInterface);
}

static void draw_menu(BadUsbApp* bad_usb) {
    VariableItemList* var_item_list = bad_usb->var_item_list;

    variable_item_list_reset(var_item_list);

    variable_item_list_add(var_item_list, "Keyboard Layout (global)", 0, NULL, NULL);

    VariableItem* item = variable_item_list_add(
        var_item_list, "Interface", 2, bad_usb_scene_config_interface_callback, bad_usb);
    if(bad_usb->interface == BadUsbHidInterfaceUsb) {
        variable_item_set_current_value_index(item, 0);
        variable_item_set_current_value_text(item, interface_mode_text[0]);
    } else {
        variable_item_set_current_value_index(item, 1);
        variable_item_set_current_value_text(item, interface_mode_text[1]);
        variable_item_list_add(var_item_list, "Remove Pairing", 0, NULL, NULL);
    }
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
        } else if(event.event == ConfigIndexInterface) {
            draw_menu(bad_usb);
        } else if(event.event == ConfigIndexBleUnpair) {
            bad_usb_hid_ble_remove_pairing();
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
