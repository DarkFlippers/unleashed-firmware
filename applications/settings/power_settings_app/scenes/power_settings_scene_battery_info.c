#include "../power_settings_app.h"

static void power_settings_scene_battery_info_update_model(PowerSettingsApp* app) {
    power_get_info(app->power, &app->info);
    BatteryInfoModel battery_info_data = {
        .vbus_voltage = app->info.voltage_vbus,
        .gauge_voltage = app->info.voltage_gauge,
        .gauge_current = app->info.current_gauge,
        .gauge_temperature = app->info.temperature_gauge,
        .charge_voltage_limit = app->info.voltage_battery_charge_limit,
        .charge = app->info.charge,
        .health = app->info.health,
    };
    battery_info_set_data(app->batery_info, &battery_info_data);
}

void power_settings_scene_battery_info_on_enter(void* context) {
    PowerSettingsApp* app = context;
    power_settings_scene_battery_info_update_model(app);
    view_dispatcher_switch_to_view(app->view_dispatcher, PowerSettingsAppViewBatteryInfo);
}

bool power_settings_scene_battery_info_on_event(void* context, SceneManagerEvent event) {
    PowerSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeTick) {
        power_settings_scene_battery_info_update_model(app);
        consumed = true;
    }
    return consumed;
}

void power_settings_scene_battery_info_on_exit(void* context) {
    UNUSED(context);
}
