#include <applications.h>
#include <lib/toolbox/value_index.h>

#include "../desktop_settings_app.h"
#include "desktop_settings_scene.h"
#include <power/power_service/power.h>

#define SCENE_EVENT_SELECT_FAVORITE_PRIMARY 0
#define SCENE_EVENT_SELECT_FAVORITE_SECONDARY 1
#define SCENE_EVENT_SELECT_FAVORITE_TERTIARY 2
#define SCENE_EVENT_SELECT_PIN_SETUP 3
#define SCENE_EVENT_SELECT_AUTO_LOCK_DELAY 4
#define SCENE_EVENT_SELECT_BATTERY_DISPLAY 5
#define SCENE_EVENT_SELECT_CLOCK_DISPLAY 6
#define SCENE_EVENT_SELECT_CHANGE_NAME 7

#define AUTO_LOCK_DELAY_COUNT 9
const char* const auto_lock_delay_text[AUTO_LOCK_DELAY_COUNT] = {
    "OFF",
    "10s",
    "15s",
    "30s",
    "60s",
    "90s",
    "2min",
    "5min",
    "10min",
};

const uint32_t auto_lock_delay_value[AUTO_LOCK_DELAY_COUNT] =
    {0, 10000, 15000, 30000, 60000, 90000, 120000, 300000, 600000};

#define CLOCK_ENABLE_COUNT 2
const char* const clock_enable_text[CLOCK_ENABLE_COUNT] = {
    "OFF",
    "ON",
};

const uint32_t clock_enable_value[CLOCK_ENABLE_COUNT] = {0, 1};

#define BATTERY_VIEW_COUNT 6

const char* const battery_view_count_text[BATTERY_VIEW_COUNT] =
    {"Bar", "%", "Inv. %", "Retro 3", "Retro 5", "Bar %"};

const uint32_t displayBatteryPercentage_value[BATTERY_VIEW_COUNT] = {
    DISPLAY_BATTERY_BAR,
    DISPLAY_BATTERY_PERCENT,
    DISPLAY_BATTERY_INVERTED_PERCENT,
    DISPLAY_BATTERY_RETRO_3,
    DISPLAY_BATTERY_RETRO_5,
    DISPLAY_BATTERY_BAR_PERCENT};

static void desktop_settings_scene_start_var_list_enter_callback(void* context, uint32_t index) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void desktop_settings_scene_start_battery_view_changed(VariableItem* item) {
    DesktopSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, battery_view_count_text[index]);
    app->settings.displayBatteryPercentage = index;
}

static void desktop_settings_scene_start_clock_enable_changed(VariableItem* item) {
    DesktopSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, clock_enable_text[index]);
    app->settings.display_clock = index;
}

static void desktop_settings_scene_start_auto_lock_delay_changed(VariableItem* item) {
    DesktopSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, auto_lock_delay_text[index]);
    app->settings.auto_lock_delay_ms = auto_lock_delay_value[index];
}

void desktop_settings_scene_start_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    VariableItemList* variable_item_list = app->variable_item_list;

    VariableItem* item;
    uint8_t value_index;

    variable_item_list_add(variable_item_list, "Primary Favorite App", 1, NULL, NULL);

    variable_item_list_add(variable_item_list, "Secondary Favorite App", 1, NULL, NULL);

    variable_item_list_add(variable_item_list, "Tertiary Favorite App", 1, NULL, NULL);

    variable_item_list_add(variable_item_list, "PIN Setup", 1, NULL, NULL);

    item = variable_item_list_add(
        variable_item_list,
        "Auto Lock Time",
        AUTO_LOCK_DELAY_COUNT,
        desktop_settings_scene_start_auto_lock_delay_changed,
        app);

    variable_item_list_set_enter_callback(
        variable_item_list, desktop_settings_scene_start_var_list_enter_callback, app);
    value_index = value_index_uint32(
        app->settings.auto_lock_delay_ms, auto_lock_delay_value, AUTO_LOCK_DELAY_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, auto_lock_delay_text[value_index]);

    item = variable_item_list_add(
        variable_item_list,
        "Battery View",
        BATTERY_VIEW_COUNT,
        desktop_settings_scene_start_battery_view_changed,
        app);

    value_index = value_index_uint32(
        app->settings.displayBatteryPercentage,
        displayBatteryPercentage_value,
        BATTERY_VIEW_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, battery_view_count_text[value_index]);

    item = variable_item_list_add(
        variable_item_list,
        "Show Clock",
        CLOCK_ENABLE_COUNT,
        desktop_settings_scene_start_clock_enable_changed, //
        app);

    value_index =
        value_index_uint32(app->settings.display_clock, clock_enable_value, CLOCK_ENABLE_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, clock_enable_text[value_index]);

    variable_item_list_add(variable_item_list, "Change Flipper Name", 0, NULL, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewVarItemList);
}

bool desktop_settings_scene_start_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case SCENE_EVENT_SELECT_FAVORITE_PRIMARY:
            scene_manager_set_scene_state(app->scene_manager, DesktopSettingsAppSceneFavorite, 0);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_FAVORITE_SECONDARY:
            scene_manager_set_scene_state(app->scene_manager, DesktopSettingsAppSceneFavorite, 1);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_FAVORITE_TERTIARY:
            scene_manager_set_scene_state(app->scene_manager, DesktopSettingsAppSceneFavorite, 2);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_PIN_SETUP:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinMenu);
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_AUTO_LOCK_DELAY:
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_BATTERY_DISPLAY:
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_CLOCK_DISPLAY:
            consumed = true;
            break;
        case SCENE_EVENT_SELECT_CHANGE_NAME:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneChangeName);
            consumed = true;
            break;
        }
    }
    return consumed;
}

void desktop_settings_scene_start_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    variable_item_list_reset(app->variable_item_list);
    DESKTOP_SETTINGS_SAVE(&app->settings);

    // Trigger UI update in case we changed battery layout
    Power* power = furi_record_open(RECORD_POWER);
    power_trigger_ui_update(power);
    furi_record_close(RECORD_POWER);
}
