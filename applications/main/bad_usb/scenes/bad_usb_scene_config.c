#include "../bad_usb_app_i.h"

enum ConfigIndex {
    ConfigIndexKeyboardLayout,
    ConfigIndexConnection,
};

enum ConfigIndexBle {
    ConfigIndexBlePersistPairing = ConfigIndexConnection + 1,
    ConfigIndexBlePairingMode,
    ConfigIndexBleSetDeviceName,
    ConfigIndexBleSetMacAddress,
    ConfigIndexBleRandomizeMacAddress,
    ConfigIndexBleRestoreDefaults,
    ConfigIndexBleRemovePairing,
};

enum ConfigIndexUsb {
    ConfigIndexUsbSetManufacturerName = ConfigIndexConnection + 1,
    ConfigIndexUsbSetProductName,
    ConfigIndexUsbSetVidPid,
    ConfigIndexUsbRandomizeVidPid,
    ConfigIndexUsbRestoreDefaults,
};

void bad_usb_scene_config_connection_callback(VariableItem* item) {
    BadUsbApp* bad_usb = variable_item_get_context(item);
    bad_usb_set_interface(
        bad_usb,
        bad_usb->interface == BadUsbHidInterfaceBle ? BadUsbHidInterfaceUsb :
                                                      BadUsbHidInterfaceBle);
    variable_item_set_current_value_text(
        item, bad_usb->interface == BadUsbHidInterfaceBle ? "BLE" : "USB");
    view_dispatcher_send_custom_event(bad_usb->view_dispatcher, ConfigIndexConnection);
}

void bad_usb_scene_config_ble_persist_pairing_callback(VariableItem* item) {
    BadUsbApp* bad_usb = variable_item_get_context(item);
    bool value = variable_item_get_current_value_index(item);
    const BadUsbHidApi* hid = bad_usb_hid_get_interface(bad_usb->interface);
    // Apply to current script config
    bad_usb->script_hid_cfg.ble.bonding = value;
    hid->adjust_config(&bad_usb->script_hid_cfg);
    // Set in user config to save in settings file
    bad_usb->user_hid_cfg.ble.bonding = value;
    variable_item_set_current_value_text(item, value ? "ON" : "OFF");
}

const char* const ble_pairing_mode_names[GapPairingCount] = {
    "YesNo",
    "PIN Type",
    "PIN Y/N",
};
void bad_usb_scene_config_ble_pairing_mode_callback(VariableItem* item) {
    BadUsbApp* bad_usb = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    const BadUsbHidApi* hid = bad_usb_hid_get_interface(bad_usb->interface);
    // Apply to current script config
    bad_usb->script_hid_cfg.ble.pairing = index;
    hid->adjust_config(&bad_usb->script_hid_cfg);
    // Set in user config to save in settings file
    bad_usb->user_hid_cfg.ble.pairing = index;
    variable_item_set_current_value_text(item, ble_pairing_mode_names[index]);
}

void bad_usb_scene_config_select_callback(void* context, uint32_t index) {
    BadUsbApp* bad_usb = context;

    view_dispatcher_send_custom_event(bad_usb->view_dispatcher, index);
}

static void draw_menu(BadUsbApp* bad_usb) {
    VariableItemList* var_item_list = bad_usb->var_item_list;
    VariableItem* item;

    variable_item_list_reset(var_item_list);

    variable_item_list_add(var_item_list, "Keyboard Layout (global)", 0, NULL, NULL);

    item = variable_item_list_add(
        var_item_list, "Connection", 2, bad_usb_scene_config_connection_callback, bad_usb);
    variable_item_set_current_value_index(item, bad_usb->interface == BadUsbHidInterfaceBle);
    variable_item_set_current_value_text(
        item, bad_usb->interface == BadUsbHidInterfaceBle ? "BLE" : "USB");

    if(bad_usb->interface == BadUsbHidInterfaceBle) {
        BleProfileHidParams* ble_hid_cfg = &bad_usb->script_hid_cfg.ble;

        item = variable_item_list_add(
            var_item_list,
            "Persist Pairing",
            2,
            bad_usb_scene_config_ble_persist_pairing_callback,
            bad_usb);
        variable_item_set_current_value_index(item, ble_hid_cfg->bonding);
        variable_item_set_current_value_text(item, ble_hid_cfg->bonding ? "ON" : "OFF");

        item = variable_item_list_add(
            var_item_list,
            "Pairing Mode",
            GapPairingCount,
            bad_usb_scene_config_ble_pairing_mode_callback,
            bad_usb);
        variable_item_set_current_value_index(item, ble_hid_cfg->pairing);
        variable_item_set_current_value_text(item, ble_pairing_mode_names[ble_hid_cfg->pairing]);

        variable_item_list_add(var_item_list, "Set Device Name", 0, NULL, NULL);

        variable_item_list_add(var_item_list, "Set MAC Address", 0, NULL, NULL);

        variable_item_list_add(var_item_list, "Randomize MAC Address", 0, NULL, NULL);

        variable_item_list_add(var_item_list, "Restore BLE Defaults", 0, NULL, NULL);

        variable_item_list_add(var_item_list, "Remove BLE Pairing", 0, NULL, NULL);
    } else {
        variable_item_list_add(var_item_list, "Set Manufacturer Name", 0, NULL, NULL);

        variable_item_list_add(var_item_list, "Set Product Name", 0, NULL, NULL);

        variable_item_list_add(var_item_list, "Set VID and PID", 0, NULL, NULL);

        variable_item_list_add(var_item_list, "Randomize VID and PID", 0, NULL, NULL);

        variable_item_list_add(var_item_list, "Restore USB Defaults", 0, NULL, NULL);
    }
}

void bad_usb_scene_config_on_enter(void* context) {
    BadUsbApp* bad_usb = context;
    VariableItemList* var_item_list = bad_usb->var_item_list;

    variable_item_list_set_enter_callback(
        var_item_list, bad_usb_scene_config_select_callback, bad_usb);
    draw_menu(bad_usb);
    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(bad_usb->scene_manager, BadUsbSceneConfig));

    view_dispatcher_switch_to_view(bad_usb->view_dispatcher, BadUsbAppViewConfig);
}

bool bad_usb_scene_config_on_event(void* context, SceneManagerEvent event) {
    BadUsbApp* bad_usb = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(bad_usb->scene_manager, BadUsbSceneConfig, event.event);
        consumed = true;
        const BadUsbHidApi* hid = bad_usb_hid_get_interface(bad_usb->interface);

        switch(event.event) {
        case ConfigIndexKeyboardLayout:
            scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneConfigLayout);
            break;
        case ConfigIndexConnection:
            // Refresh default values for new interface
            hid->adjust_config(&bad_usb->script_hid_cfg);
            // Redraw menu with new interface options
            draw_menu(bad_usb);
            break;
        default:
            break;
        }
        if(bad_usb->interface == BadUsbHidInterfaceBle) {
            switch(event.event) {
            case ConfigIndexBleSetDeviceName:
                scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneConfigBleName);
                break;
            case ConfigIndexBleSetMacAddress:
                scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneConfigBleMac);
                break;
            case ConfigIndexBleRandomizeMacAddress:
                // Apply to current script config
                furi_hal_random_fill_buf(
                    bad_usb->script_hid_cfg.ble.mac, sizeof(bad_usb->script_hid_cfg.ble.mac));
                bad_usb->script_hid_cfg.ble.mac[sizeof(bad_usb->script_hid_cfg.ble.mac) - 1] |=
                    0b11 << 6; // Set 2 MSB for Random Static Address
                hid->adjust_config(&bad_usb->script_hid_cfg);
                // Set in user config to save in settings file
                memcpy(
                    bad_usb->user_hid_cfg.ble.mac,
                    bad_usb->script_hid_cfg.ble.mac,
                    sizeof(bad_usb->user_hid_cfg.ble.mac));
                scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneDone);
                break;
            case ConfigIndexBleRestoreDefaults:
                // Apply to current script config
                bad_usb->script_hid_cfg.ble.name[0] = '\0';
                memset(
                    bad_usb->script_hid_cfg.ble.mac, 0, sizeof(bad_usb->script_hid_cfg.ble.mac));
                bad_usb->script_hid_cfg.ble.bonding = true;
                bad_usb->script_hid_cfg.ble.pairing = GapPairingPinCodeVerifyYesNo;
                hid->adjust_config(&bad_usb->script_hid_cfg);
                // Set in user config to save in settings file
                memcpy(
                    &bad_usb->user_hid_cfg.ble,
                    &bad_usb->script_hid_cfg.ble,
                    sizeof(bad_usb->user_hid_cfg.ble));
                scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneDone);
                break;
            case ConfigIndexBleRemovePairing:
                scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneConfirmUnpair);
                break;
            default:
                break;
            }
        } else {
            switch(event.event) {
            case ConfigIndexUsbSetManufacturerName:
                scene_manager_set_scene_state(
                    bad_usb->scene_manager, BadUsbSceneConfigUsbName, true);
                scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneConfigUsbName);
                break;
            case ConfigIndexUsbSetProductName:
                scene_manager_set_scene_state(
                    bad_usb->scene_manager, BadUsbSceneConfigUsbName, false);
                scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneConfigUsbName);
                break;
            case ConfigIndexUsbSetVidPid:
                scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneConfigUsbVidPid);
                break;
            case ConfigIndexUsbRandomizeVidPid:
                furi_hal_random_fill_buf(
                    (void*)bad_usb->usb_vidpid_buf, sizeof(bad_usb->usb_vidpid_buf));
                // Apply to current script config
                bad_usb->script_hid_cfg.usb.vid = bad_usb->usb_vidpid_buf[0];
                bad_usb->script_hid_cfg.usb.pid = bad_usb->usb_vidpid_buf[1];
                hid->adjust_config(&bad_usb->script_hid_cfg);
                // Set in user config to save in settings file
                bad_usb->user_hid_cfg.usb.vid = bad_usb->script_hid_cfg.usb.vid;
                bad_usb->user_hid_cfg.usb.pid = bad_usb->script_hid_cfg.usb.pid;
                scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneDone);
                break;
            case ConfigIndexUsbRestoreDefaults:
                // Apply to current script config
                bad_usb->script_hid_cfg.usb.vid = 0;
                bad_usb->script_hid_cfg.usb.pid = 0;
                bad_usb->script_hid_cfg.usb.manuf[0] = '\0';
                bad_usb->script_hid_cfg.usb.product[0] = '\0';
                hid->adjust_config(&bad_usb->script_hid_cfg);
                // Set in user config to save in settings file
                memcpy(
                    &bad_usb->user_hid_cfg.usb,
                    &bad_usb->script_hid_cfg.usb,
                    sizeof(bad_usb->user_hid_cfg.usb));
                scene_manager_next_scene(bad_usb->scene_manager, BadUsbSceneDone);
                break;
            default:
                break;
            }
        }
    }

    return consumed;
}

void bad_usb_scene_config_on_exit(void* context) {
    BadUsbApp* bad_usb = context;
    VariableItemList* var_item_list = bad_usb->var_item_list;

    variable_item_list_reset(var_item_list);
}
