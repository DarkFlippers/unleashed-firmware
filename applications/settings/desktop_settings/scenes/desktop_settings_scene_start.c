#include <applications.h>
#include <lib/toolbox/value_index.h>

#include "../desktop_settings_app.h"
#include "desktop_settings_scene.h"

#define SCENE_EVENT_SELECT_FAVORITE_PRIMARY 0
#define SCENE_EVENT_SELECT_FAVORITE_SECONDARY 1
// #define SCENE_EVENT_SELECT_FAVORITE_GAME 2
#define SCENE_EVENT_SELECT_PIN_SETUP 2
#define SCENE_EVENT_SELECT_AUTO_LOCK_DELAY 3
#define SCENE_EVENT_SELECT_BATTERY_DISPLAY 4
#define SCENE_EVENT_SELECT_DUMBMODE 5

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

#define BATTERY_VIEW_COUNT 5
const char* const battery_view_count_text[BATTERY_VIEW_COUNT] = {
    "Bar",
    "%",
    "Inv. %",
    "Retro 3",
    "Retro 5",
};
const uint32_t displayBatteryPercentage_value[BATTERY_VIEW_COUNT] = {0, 1, 2, 3, 4};

uint8_t origBattDisp_value = 0;

#define DUMBMODE_COUNT 2
const char* const dumbmode_text[DUMBMODE_COUNT] = {
    "OFF",
    "ON",
};

const uint32_t dumbmode_value[DUMBMODE_COUNT] = {0, 1};

static void desktop_settings_scene_start_var_list_enter_callback(void* context, uint32_t index) {
    DesktopSettingsApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

static void desktop_settings_scene_start_auto_lock_delay_changed(VariableItem* item) {
    DesktopSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, auto_lock_delay_text[index]);
    app->settings.auto_lock_delay_ms = auto_lock_delay_value[index];
}

static void desktop_settings_scene_start_battery_view_changed(VariableItem* item) {
    DesktopSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, battery_view_count_text[index]);
    app->settings.displayBatteryPercentage = index;
}

static void desktop_settings_scene_start_dumbmode_changed(VariableItem* item) {
    DesktopSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, dumbmode_text[index]);
    app->settings.is_dumbmode = dumbmode_value[index];
}

void desktop_settings_scene_start_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    VariableItemList* variable_item_list = app->variable_item_list;
    origBattDisp_value = app->settings.displayBatteryPercentage;

    VariableItem* item;
    uint8_t value_index;

    variable_item_list_add(variable_item_list, "Primary Favorite App", 1, NULL, NULL);

    variable_item_list_add(variable_item_list, "Secondary Favorite App", 1, NULL, NULL);

    // variable_item_list_add(variable_item_list, "Favorite Game", 1, NULL, NULL);

    variable_item_list_add(variable_item_list, "PIN Setup", 1, NULL, NULL);

    item = variable_item_list_add(
        variable_item_list,
        "Auto Lock Time",
        AUTO_LOCK_DELAY_COUNT,
        desktop_settings_scene_start_auto_lock_delay_changed,
        app);

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
        "Games Only",
        DUMBMODE_COUNT,
        desktop_settings_scene_start_dumbmode_changed,
        app);

    value_index = value_index_uint32(app->settings.is_dumbmode, dumbmode_value, DUMBMODE_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, dumbmode_text[value_index]);

    variable_item_list_set_enter_callback(
        variable_item_list, desktop_settings_scene_start_var_list_enter_callback, app);
    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewVarItemList);
}

bool desktop_settings_scene_start_on_event(void* context, SceneManagerEvent sme) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(sme.type == SceneManagerEventTypeCustom) {
        switch(sme.event) {
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
        // case SCENE_EVENT_SELECT_FAVORITE_GAME:
        // scene_manager_set_scene_state(app->scene_manager, DesktopSettingsAppSceneFavorite, 2);
        // scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
        // consumed = true;
        // break;
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
        case SCENE_EVENT_SELECT_DUMBMODE:
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

    if(app->settings.displayBatteryPercentage != origBattDisp_value) {
        furi_hal_power_reset();
    }
}
