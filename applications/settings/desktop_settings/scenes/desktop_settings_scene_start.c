#include <applications.h>
#include <lib/toolbox/value_index.h>
#include <storage/storage.h>
#include <flipper_application/flipper_application.h>
#include <loader/loader.h>

#include "../desktop_settings_app.h"
#include "desktop_settings_scene.h"
#include "desktop_settings_scene_i.h"
#include <power/power_service/power.h>

#define TAG "DesktopSettings"

// variable_item_list_add() takes a uint8_t values count, and on_enter passes menu_styles_count
// plus one for "Default" - this is what stops that from wrapping
#define MENU_STYLES_MAX (UINT8_MAX - 1)

typedef enum {
    DesktopSettingsPinSetup = 0,
    DesktopSettingsAutoLockDelay,
    DesktopSettingsAutoPowerOff,
    DesktopSettingsBatteryDisplay,
    DesktopSettingsClockDisplay,
    DesktopSettingsMenuStyle,
    DesktopSettingsChangeName,
    DesktopSettingsHappyMode,
    DesktopSettingsFavoriteLeftShort,
    DesktopSettingsFavoriteLeftLong,
    DesktopSettingsFavoriteRightShort,
    DesktopSettingsFavoriteRightLong,
    DesktopSettingsFavoriteOkLong,
    DesktopSettingsDummyLeft,
    DesktopSettingsDummyLeftLong,
    DesktopSettingsDummyRight,
    DesktopSettingsDummyRightLong,
    DesktopSettingsDummyUpLong,
    DesktopSettingsDummyDown,
    DesktopSettingsDummyDownLong,
    DesktopSettingsDummyOk,
    DesktopSettingsDummyOkLong,
} DesktopSettingsEntry;

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

#define USB_INHIBIT_AUTO_LOCK_DELAY_COUNT 2

const char* const usb_inhibit_auto_lock_delay_text[USB_INHIBIT_AUTO_LOCK_DELAY_COUNT] = {
    "OFF",
    "ON",
};

const uint32_t usb_inhibit_auto_lock_delay_value[USB_INHIBIT_AUTO_LOCK_DELAY_COUNT] = {0, 1};

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

static void desktop_settings_scene_start_menu_styles_load(DesktopSettingsApp* app) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* dir = storage_file_alloc(storage);
    FuriString* path = furi_string_alloc();
    FuriString* name = furi_string_alloc();
    FuriString* file_name = furi_string_alloc();
    char file[64];
    uint8_t icon[FAP_MANIFEST_MAX_ICON_SIZE];
    uint8_t* icon_ptr = icon;

    if(storage_dir_open(dir, LOADER_MENU_STYLES_PATH)) {
        while(storage_dir_read(dir, NULL, file, sizeof(file))) {
            // The directory holds every loader plugin, so filter on the menu style appid prefix
            furi_string_set_str(file_name, file);
            if(!furi_string_start_with_str(file_name, LOADER_MENU_STYLE_PREFIX) ||
               !furi_string_end_with_str(file_name, ".fal")) {
                continue;
            }
            // This one is a menu style we cannot offer, rather than something we filtered out
            if(furi_string_size(file_name) >= sizeof(app->settings.menu_style)) {
                FURI_LOG_W(TAG, "Menu style name too long, ignoring %s", file);
                continue;
            }
            if(app->menu_styles_count >= MENU_STYLES_MAX) {
                FURI_LOG_W(TAG, "More than %u menu styles, ignoring the rest", MENU_STYLES_MAX);
                break;
            }
            furi_string_printf(path, "%s/%s", LOADER_MENU_STYLES_PATH, file);
            if(!flipper_application_load_name_and_icon(path, storage, &icon_ptr, name)) {
                FURI_LOG_W(TAG, "Skipping unreadable menu style %s", file);
                continue;
            }
            size_t pos = app->menu_styles_count;
            app->menu_styles =
                realloc(app->menu_styles, (pos + 1) * sizeof(DesktopSettingsMenuStyleEntry));
            while(pos && furi_string_cmp(app->menu_styles[pos - 1].name, name) > 0) {
                app->menu_styles[pos] = app->menu_styles[pos - 1];
                pos--;
            }
            app->menu_styles[pos].file = furi_string_alloc_set_str(file);
            app->menu_styles[pos].name = furi_string_alloc_set(name);
            app->menu_styles_count++;
        }
    } else {
        FURI_LOG_D(TAG, "No menu styles in %s", LOADER_MENU_STYLES_PATH);
    }

    storage_dir_close(dir);
    storage_file_free(dir);
    furi_string_free(path);
    furi_string_free(name);
    furi_string_free(file_name);
    furi_record_close(RECORD_STORAGE);
}

static void desktop_settings_scene_start_menu_styles_free(DesktopSettingsApp* app) {
    for(size_t i = 0; i < app->menu_styles_count; i++) {
        furi_string_free(app->menu_styles[i].file);
        furi_string_free(app->menu_styles[i].name);
    }
    free(app->menu_styles);
    app->menu_styles = NULL;
    app->menu_styles_count = 0;
}

static void desktop_settings_scene_start_menu_style_changed(VariableItem* item) {
    DesktopSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    if(index == 0) {
        variable_item_set_current_value_text(item, "Default");
        app->settings.menu_style[0] = '\0';
    } else {
        const DesktopSettingsMenuStyleEntry* style = &app->menu_styles[index - 1];
        variable_item_set_current_value_text(item, furi_string_get_cstr(style->name));
        strlcpy(
            app->settings.menu_style,
            furi_string_get_cstr(style->file),
            sizeof(app->settings.menu_style));
    }
}

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

static void desktop_settings_scene_start_usb_inhibit_auto_lock_delay_changed(VariableItem* item) {
    DesktopSettingsApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, usb_inhibit_auto_lock_delay_text[index]);
    app->settings.usb_inhibit_auto_lock = usb_inhibit_auto_lock_delay_value[index];
}

void desktop_settings_scene_start_on_enter(void* context) {
    DesktopSettingsApp* app = context;
    VariableItemList* variable_item_list = app->variable_item_list;

    VariableItem* item;
    uint8_t value_index;

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

    // USB connection Inhibit autolock OFF|ON|with opened RPC session
    item = variable_item_list_add(
        variable_item_list,
        "Auto Lock disarm by active USB session",
        USB_INHIBIT_AUTO_LOCK_DELAY_COUNT,
        desktop_settings_scene_start_usb_inhibit_auto_lock_delay_changed,
        app);

    value_index = value_index_uint32(
        app->settings.usb_inhibit_auto_lock,
        usb_inhibit_auto_lock_delay_value,
        USB_INHIBIT_AUTO_LOCK_DELAY_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, usb_inhibit_auto_lock_delay_text[value_index]);

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
        desktop_settings_scene_start_clock_enable_changed,
        app);

    value_index =
        value_index_uint32(app->settings.display_clock, clock_enable_value, CLOCK_ENABLE_COUNT);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, clock_enable_text[value_index]);

    desktop_settings_scene_start_menu_styles_load(app);
    item = variable_item_list_add(
        variable_item_list,
        "Menu Style",
        app->menu_styles_count + 1,
        desktop_settings_scene_start_menu_style_changed,
        app);

    value_index = 0;
    for(size_t i = 0; i < app->menu_styles_count; i++) {
        if(furi_string_equal_str(app->menu_styles[i].file, app->settings.menu_style)) {
            value_index = i + 1;
            break;
        }
    }
    variable_item_set_current_value_index(item, value_index);
    const char* menu_style_text = "Default";
    if(value_index) {
        menu_style_text = furi_string_get_cstr(app->menu_styles[value_index - 1].name);
    } else if(app->settings.menu_style[0]) {
        // Configured style is gone - do not present that as having chosen the built-in one
        menu_style_text = "Missing";
    }
    variable_item_set_current_value_text(item, menu_style_text);

    variable_item_list_add(variable_item_list, "Change Flipper Name", 0, NULL, app);

    variable_item_list_add(variable_item_list, "Happy Mode", 1, NULL, NULL);

    variable_item_list_add(variable_item_list, "Favorite App - Left Short", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "Favorite App - Left Long", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "Favorite App - Right Short", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "Favorite App - Right Long", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "Favorite App - Ok Long", 1, NULL, NULL);

    variable_item_list_add(variable_item_list, "DummyMode - Left", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "DummyMode - Left Long", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "DummyMode - Right", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "DummyMode - Right Long", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "DummyMode - Up Long", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "DummyMode - Down", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "DummyMode - Down Long", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "DummyMode - Ok", 1, NULL, NULL);
    variable_item_list_add(variable_item_list, "DummyMode - Ok Long", 1, NULL, NULL);

    variable_item_list_set_enter_callback(
        variable_item_list, desktop_settings_scene_start_var_list_enter_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, DesktopSettingsAppViewVarItemList);
}

bool desktop_settings_scene_start_on_event(void* context, SceneManagerEvent event) {
    DesktopSettingsApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        switch(event.event) {
        case DesktopSettingsPinSetup:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppScenePinMenu);
            break;

            // case DesktopSettingsAutoLockDelay:
            // case DesktopSettingsBatteryDisplay:
            // case DesktopSettingsClockDisplay:
            // Proces in default

        case DesktopSettingsChangeName:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneChangeName);
            break;

        case DesktopSettingsFavoriteLeftShort:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_FAVORITE_APP | FavoriteAppLeftShort);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsFavoriteLeftLong:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_FAVORITE_APP | FavoriteAppLeftLong);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsFavoriteRightShort:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_FAVORITE_APP | FavoriteAppRightShort);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsFavoriteRightLong:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_FAVORITE_APP | FavoriteAppRightLong);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsFavoriteOkLong:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_FAVORITE_APP | FavoriteAppOkLong);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;

        case DesktopSettingsDummyLeft:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppLeftShort);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsDummyLeftLong:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppLeftLong);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsDummyRight:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppRightShort);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsDummyRightLong:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppRightLong);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsDummyUpLong:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppUpLong);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsDummyDown:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppDownShort);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsDummyDownLong:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppDownLong);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsDummyOk:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppOkShort);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;
        case DesktopSettingsDummyOkLong:
            scene_manager_set_scene_state(
                app->scene_manager,
                DesktopSettingsAppSceneFavorite,
                SCENE_STATE_SET_DUMMY_APP | DummyAppOkLong);
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneFavorite);
            break;

        case DesktopSettingsHappyMode:
            scene_manager_next_scene(app->scene_manager, DesktopSettingsAppSceneHappyMode);
            break;

        default:
            break;
        }
        consumed = true;
    }
    return consumed;
}

void desktop_settings_scene_start_on_exit(void* context) {
    DesktopSettingsApp* app = context;
    variable_item_list_reset(app->variable_item_list);
    desktop_settings_scene_start_menu_styles_free(app);
    desktop_settings_save(&app->settings);

    // Trigger UI update in case we changed battery layout
    Power* power = furi_record_open(RECORD_POWER);
    power_trigger_ui_update(power);
    furi_record_close(RECORD_POWER);
}
