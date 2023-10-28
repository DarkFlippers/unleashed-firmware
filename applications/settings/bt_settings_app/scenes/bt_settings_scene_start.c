#include "../bt_settings_app.h"
#include <furi_hal_bt.h>

enum BtSetting {
    BtSettingOff,
    BtSettingOn,
    BtSettingNum,
};

enum BtSettingIndex {
    BtSettingIndexSwitchBt,
    BtSettingIndexSwitchAdv,
    BtSettingIndexForgetDev,
};

const char* const bt_settings_text[BtSettingNum] = {
    "OFF",
    "ON",
};

const char* const bt_advertise_text[BtAdvNum] = {
    "All",
    "Name",
};

static void bt_settings_scene_start_set_switch_callback(VariableItem* item) {
    BtSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, bt_settings_text[index]);
    if(index == BtSettingOff) {
        view_dispatcher_send_custom_event(app->view_dispatcher, BtSettingsCustomEventBtDisable);
    } else {
        view_dispatcher_send_custom_event(app->view_dispatcher, BtSettingsCustomEventBtEnable);
    }
}

static void bt_settings_scene_start_set_advertise_callback(VariableItem* item) {
    BtSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, bt_advertise_text[index]);
    app->settings.advertise_type = index;
    view_dispatcher_send_custom_event(app->view_dispatcher, BtSettingsCustomEventBtEnable);
}

static void bt_settings_scene_start_var_list_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    BtSettingsApp* app = context;
    if(index == BtSettingIndexForgetDev) {
        view_dispatcher_send_custom_event(
            app->view_dispatcher, BtSettingsCustomEventForgetDevices);
    }
}

void bt_settings_scene_start_on_enter(void* context) {
    BtSettingsApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;
    VariableItem* item;

    if(furi_hal_bt_is_ble_gatt_gap_supported()) {
        item = variable_item_list_add(
            var_item_list,
            "Bluetooth",
            BtSettingNum,
            bt_settings_scene_start_set_switch_callback,
            app);
        variable_item_set_current_value_index(item, app->settings.enabled);
        variable_item_set_current_value_text(item, bt_settings_text[app->settings.enabled]);

        item = variable_item_list_add(
            var_item_list,
            "Advertise",
            BtAdvNum,
            bt_settings_scene_start_set_advertise_callback,
            app);
        variable_item_set_current_value_index(item, app->settings.advertise_type);
        variable_item_set_current_value_text(
            item, bt_advertise_text[app->settings.advertise_type]);
        variable_item_set_locked(item, !app->settings.enabled, "Enable BT!");

        variable_item_list_add(var_item_list, "Forget All Paired Devices", 1, NULL, NULL);
        variable_item_list_set_enter_callback(
            var_item_list, bt_settings_scene_start_var_list_enter_callback, app);
    } else {
        item = variable_item_list_add(var_item_list, "Bluetooth", 1, NULL, NULL);
        variable_item_set_current_value_text(item, "Broken");
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, BtSettingsAppViewVarItemList);
}

bool bt_settings_scene_start_on_event(void* context, SceneManagerEvent event) {
    BtSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BtSettingsCustomEventBtEnable) {
            if(app->settings.advertise_type == BtAdvAll) {
                furi_hal_bt_set_profile_adv_name(
                    FuriHalBtProfileSerial, furi_hal_version_get_device_name_ptr());
            } else {
                const char* advname = "\0";
                if(app->settings.advertise_type == BtAdvName) {
                    advname = furi_hal_version_get_name_ptr();
                }
                furi_hal_bt_set_profile_adv_name(FuriHalBtProfileSerial, advname);
            }
            furi_hal_bt_stop_advertising();
            furi_hal_bt_start_advertising();
            app->settings.enabled = true;
            scene_manager_previous_scene(app->scene_manager);
            scene_manager_next_scene(app->scene_manager, BtSettingsAppSceneStart);
            consumed = true;
        } else if(event.event == BtSettingsCustomEventBtDisable) {
            app->settings.enabled = false;
            furi_hal_bt_stop_advertising();
            scene_manager_previous_scene(app->scene_manager);
            scene_manager_next_scene(app->scene_manager, BtSettingsAppSceneStart);
            consumed = true;
        } else if(event.event == BtSettingsCustomEventForgetDevices) {
            scene_manager_next_scene(app->scene_manager, BtSettingsAppSceneForgetDevConfirm);
            consumed = true;
        }
    }
    return consumed;
}

void bt_settings_scene_start_on_exit(void* context) {
    BtSettingsApp* app = context;
    variable_item_list_reset(app->var_item_list);
}
