#include "../bad_duck3_app_i.h"

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

void bad_duck3_scene_config_connection_callback(VariableItem* item) {
    BadDuck3App* app = variable_item_get_context(item);
    bad_duck3_set_interface(
        app,
        app->interface == Duck3HidInterfaceBle ? Duck3HidInterfaceUsb :
                                                 Duck3HidInterfaceBle);
    variable_item_set_current_value_text(
        item, app->interface == Duck3HidInterfaceBle ? "BLE" : "USB");
    view_dispatcher_send_custom_event(app->view_dispatcher, ConfigIndexConnection);
}

void bad_duck3_scene_config_ble_persist_pairing_callback(VariableItem* item) {
    BadDuck3App* app = variable_item_get_context(item);
    bool value = variable_item_get_current_value_index(item);
    const Duck3HidApi* hid = duck3_hid_get_interface(app->interface);
    app->script_hid_cfg.ble.bonding = value;
    hid->adjust_config(&app->script_hid_cfg);
    app->user_hid_cfg.ble.bonding = value;
    variable_item_set_current_value_text(item, value ? "ON" : "OFF");
}

const char* const ble_pairing_mode_names[GapPairingCount] = {
    "YesNo",
    "PIN Type",
    "PIN Y/N",
};

void bad_duck3_scene_config_ble_pairing_mode_callback(VariableItem* item) {
    BadDuck3App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    const Duck3HidApi* hid = duck3_hid_get_interface(app->interface);
    app->script_hid_cfg.ble.pairing = index;
    hid->adjust_config(&app->script_hid_cfg);
    app->user_hid_cfg.ble.pairing = index;
    variable_item_set_current_value_text(item, ble_pairing_mode_names[index]);
}

void bad_duck3_scene_config_select_callback(void* context, uint32_t index) {
    BadDuck3App* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void draw_menu(BadDuck3App* app) {
    VariableItemList* var_item_list = app->var_item_list;
    VariableItem* item;

    variable_item_list_reset(var_item_list);

    variable_item_list_add(var_item_list, "Keyboard Layout (global)", 0, NULL, NULL);

    item = variable_item_list_add(
        var_item_list, "Connection", 2, bad_duck3_scene_config_connection_callback, app);
    variable_item_set_current_value_index(item, app->interface == Duck3HidInterfaceBle);
    variable_item_set_current_value_text(
        item, app->interface == Duck3HidInterfaceBle ? "BLE" : "USB");

    if(app->interface == Duck3HidInterfaceBle) {
        Duck3BleConfig* ble_hid_cfg = &app->script_hid_cfg.ble;

        item = variable_item_list_add(
            var_item_list,
            "Persist Pairing",
            2,
            bad_duck3_scene_config_ble_persist_pairing_callback,
            app);
        variable_item_set_current_value_index(item, ble_hid_cfg->bonding);
        variable_item_set_current_value_text(item, ble_hid_cfg->bonding ? "ON" : "OFF");

        item = variable_item_list_add(
            var_item_list,
            "Pairing Mode",
            GapPairingCount,
            bad_duck3_scene_config_ble_pairing_mode_callback,
            app);
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

void bad_duck3_scene_config_on_enter(void* context) {
    BadDuck3App* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    variable_item_list_set_enter_callback(
        var_item_list, bad_duck3_scene_config_select_callback, app);
    draw_menu(app);
    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, BadDuck3SceneConfig));

    view_dispatcher_switch_to_view(app->view_dispatcher, BadDuck3AppViewConfig);
}

bool bad_duck3_scene_config_on_event(void* context, SceneManagerEvent event) {
    BadDuck3App* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        scene_manager_set_scene_state(app->scene_manager, BadDuck3SceneConfig, event.event);
        consumed = true;
        const Duck3HidApi* hid = duck3_hid_get_interface(app->interface);

        switch(event.event) {
        case ConfigIndexKeyboardLayout:
            scene_manager_next_scene(app->scene_manager, BadDuck3SceneConfigLayout);
            break;
        case ConfigIndexConnection:
            hid->adjust_config(&app->script_hid_cfg);
            draw_menu(app);
            break;
        default:
            break;
        }
        if(app->interface == Duck3HidInterfaceBle) {
            switch(event.event) {
            case ConfigIndexBleSetDeviceName:
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneConfigBleName);
                break;
            case ConfigIndexBleSetMacAddress:
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneConfigBleMac);
                break;
            case ConfigIndexBleRandomizeMacAddress:
                furi_hal_random_fill_buf(
                    app->script_hid_cfg.ble.mac, sizeof(app->script_hid_cfg.ble.mac));
                app->script_hid_cfg.ble.mac[sizeof(app->script_hid_cfg.ble.mac) - 1] |=
                    0b11 << 6;
                hid->adjust_config(&app->script_hid_cfg);
                memcpy(
                    app->user_hid_cfg.ble.mac,
                    app->script_hid_cfg.ble.mac,
                    sizeof(app->user_hid_cfg.ble.mac));
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneDone);
                break;
            case ConfigIndexBleRestoreDefaults:
                app->script_hid_cfg.ble.name[0] = '\0';
                memset(
                    app->script_hid_cfg.ble.mac, 0, sizeof(app->script_hid_cfg.ble.mac));
                app->script_hid_cfg.ble.bonding = true;
                app->script_hid_cfg.ble.pairing = GapPairingPinCodeVerifyYesNo;
                hid->adjust_config(&app->script_hid_cfg);
                memcpy(
                    &app->user_hid_cfg.ble,
                    &app->script_hid_cfg.ble,
                    sizeof(app->user_hid_cfg.ble));
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneDone);
                break;
            case ConfigIndexBleRemovePairing:
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneConfirmUnpair);
                break;
            default:
                break;
            }
        } else {
            switch(event.event) {
            case ConfigIndexUsbSetManufacturerName:
                scene_manager_set_scene_state(
                    app->scene_manager, BadDuck3SceneConfigUsbName, true);
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneConfigUsbName);
                break;
            case ConfigIndexUsbSetProductName:
                scene_manager_set_scene_state(
                    app->scene_manager, BadDuck3SceneConfigUsbName, false);
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneConfigUsbName);
                break;
            case ConfigIndexUsbSetVidPid:
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneConfigUsbVidPid);
                break;
            case ConfigIndexUsbRandomizeVidPid:
                furi_hal_random_fill_buf(
                    (void*)app->usb_vidpid_buf, sizeof(app->usb_vidpid_buf));
                app->script_hid_cfg.usb.vid = app->usb_vidpid_buf[0];
                app->script_hid_cfg.usb.pid = app->usb_vidpid_buf[1];
                hid->adjust_config(&app->script_hid_cfg);
                app->user_hid_cfg.usb.vid = app->script_hid_cfg.usb.vid;
                app->user_hid_cfg.usb.pid = app->script_hid_cfg.usb.pid;
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneDone);
                break;
            case ConfigIndexUsbRestoreDefaults:
                app->script_hid_cfg.usb.vid = 0;
                app->script_hid_cfg.usb.pid = 0;
                app->script_hid_cfg.usb.manuf[0] = '\0';
                app->script_hid_cfg.usb.product[0] = '\0';
                hid->adjust_config(&app->script_hid_cfg);
                memcpy(
                    &app->user_hid_cfg.usb,
                    &app->script_hid_cfg.usb,
                    sizeof(app->user_hid_cfg.usb));
                scene_manager_next_scene(app->scene_manager, BadDuck3SceneDone);
                break;
            default:
                break;
            }
        }
    }

    return consumed;
}

void bad_duck3_scene_config_on_exit(void* context) {
    BadDuck3App* app = context;
    VariableItemList* var_item_list = app->var_item_list;
    variable_item_list_reset(var_item_list);
}
