#include "../power_settings_app.h"
#include <lib/toolbox/value_index.h>

enum PowerSettingsSubmenuIndex {
    PowerSettingsSubmenuIndexBatteryInfo,
    PowerSettingsSubmenuIndexReboot,
    PowerSettingsSubmenuIndexOff,
    PowerSettingsSubmenuIndexAutoPowerOff,
};

#define AUTO_POWEROFF_DELAY_COUNT 8
const char* const auto_poweroff_delay_text[AUTO_POWEROFF_DELAY_COUNT] =
    {"OFF", "5min", "10min", "15min", "30min", "45min", "60min", "90min"};

const uint32_t auto_poweroff_delay_value[AUTO_POWEROFF_DELAY_COUNT] =
    {0, 300000, 600000, 900000, 1800000, 2700000, 3600000, 5400000};

#define CHARGE_SUPRESS_PERCENT_COUNT 6
const char* const charge_supress_percent_text[CHARGE_SUPRESS_PERCENT_COUNT] =
    {"OFF", "90%", "85%", "80%", "75%", "70%"};

const uint32_t charge_supress_percent_value[CHARGE_SUPRESS_PERCENT_COUNT] = {0, 90, 85, 80, 75, 70};

// change variable_item_list visible text and charge_supress_percent_settings when user change item in variable_item_list
static void power_settings_scene_start_charge_supress_percent_changed(VariableItem* item) {
    PowerSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, charge_supress_percent_text[index]);
    app->settings.charge_supress_percent = charge_supress_percent_value[index];
}

// change variable_item_list visible text and app_poweroff_delay_time_settings when user change item in variable_item_list
static void power_settings_scene_start_auto_poweroff_delay_changed(VariableItem* item) {
    PowerSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, auto_poweroff_delay_text[index]);
    app->settings.auto_poweroff_delay_ms = auto_poweroff_delay_value[index];
}

static void power_settings_scene_start_submenu_callback(void* context, uint32_t index) {
    //show selected menu screen by index
    furi_assert(context);
    PowerSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void power_settings_scene_start_on_enter(void* context) {
    PowerSettingsApp* app = context;
    VariableItemList* variable_item_list = app->variable_item_list;

    variable_item_list_add(variable_item_list, "Battery Info", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "Reboot", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "Power OFF", 1, NULL, NULL);

    VariableItem* item;
    uint8_t value_index;

    item = variable_item_list_add(
        variable_item_list,
        "Auto PowerOff",
        AUTO_POWEROFF_DELAY_COUNT,
        power_settings_scene_start_auto_poweroff_delay_changed,
        app);

    value_index = value_index_uint32(
        app->settings.auto_poweroff_delay_ms,
        auto_poweroff_delay_value,
        AUTO_POWEROFF_DELAY_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, auto_poweroff_delay_text[value_index]);

    item = variable_item_list_add(
        variable_item_list,
        "Limit Charge",
        CHARGE_SUPRESS_PERCENT_COUNT,
        power_settings_scene_start_charge_supress_percent_changed,
        app);

    value_index = value_index_uint32(
        app->settings.charge_supress_percent,
        charge_supress_percent_value,
        CHARGE_SUPRESS_PERCENT_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, charge_supress_percent_text[value_index]);

    variable_item_list_set_selected_item(
        variable_item_list,
        scene_manager_get_scene_state(app->scene_manager, PowerSettingsAppSceneStart));
    variable_item_list_set_enter_callback(
        variable_item_list, power_settings_scene_start_submenu_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, PowerSettingsAppViewVariableItemList);
}

bool power_settings_scene_start_on_event(void* context, SceneManagerEvent event) {
    PowerSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == PowerSettingsSubmenuIndexBatteryInfo) {
            scene_manager_next_scene(app->scene_manager, PowerSettingsAppSceneBatteryInfo);
        } else if(event.event == PowerSettingsSubmenuIndexReboot) {
            scene_manager_next_scene(app->scene_manager, PowerSettingsAppSceneReboot);
        } else if(event.event == PowerSettingsSubmenuIndexOff) {
            scene_manager_next_scene(app->scene_manager, PowerSettingsAppScenePowerOff);
        }
        scene_manager_set_scene_state(app->scene_manager, PowerSettingsAppSceneStart, event.event);
        consumed = true;
    }
    return consumed;
}

void power_settings_scene_start_on_exit(void* context) {
    PowerSettingsApp* app = context;
    variable_item_list_reset(app->variable_item_list);
    power_settings_save(&app->settings);
}
