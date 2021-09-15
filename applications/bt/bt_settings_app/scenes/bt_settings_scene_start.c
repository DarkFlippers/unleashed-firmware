#include "../bt_settings_app.h"
#include "furi-hal-bt.h"

enum BtSetting {
    BtSettingOff,
    BtSettingOn,
    BtSettingNum,
};

const char* const bt_settings_text[BtSettingNum] = {
    "Off",
    "On",
};

static void bt_settings_scene_start_var_list_change_callback(VariableItem* item) {
    BtSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, bt_settings_text[index]);
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void bt_settings_scene_start_on_enter(void* context) {
    BtSettingsApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    VariableItem* item;
    item = variable_item_list_add(
        var_item_list,
        "Bluetooth",
        BtSettingNum,
        bt_settings_scene_start_var_list_change_callback,
        app);
    if(app->settings.enabled) {
        variable_item_set_current_value_index(item, BtSettingOn);
        variable_item_set_current_value_text(item, bt_settings_text[BtSettingOn]);
    } else {
        variable_item_set_current_value_index(item, BtSettingOff);
        variable_item_set_current_value_text(item, bt_settings_text[BtSettingOff]);
    }

    view_dispatcher_switch_to_view(app->view_dispatcher, BtSettingsAppViewVarItemList);
}

bool bt_settings_scene_start_on_event(void* context, SceneManagerEvent event) {
    BtSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == BtSettingOn) {
            furi_hal_bt_start_advertising();
            app->settings.enabled = true;
        } else if(event.event == BtSettingOff) {
            app->settings.enabled = false;
            furi_hal_bt_stop_advertising();
        }
        consumed = true;
    }
    return consumed;
}

void bt_settings_scene_start_on_exit(void* context) {
    BtSettingsApp* app = context;
    variable_item_list_clean(app->var_item_list);
}
