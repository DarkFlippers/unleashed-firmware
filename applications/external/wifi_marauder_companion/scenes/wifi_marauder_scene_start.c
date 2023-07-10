//** Includes sniffbt and sniffskim for compatible ESP32-WROOM hardware.
//wifi_marauder_app_i.h also changed **//
#include "../wifi_marauder_app_i.h"

// For each command, define whether additional arguments are needed
// (enabling text input to fill them out), and whether the console
// text box should focus at the start of the output or the end
typedef enum { NO_ARGS = 0, INPUT_ARGS, TOGGLE_ARGS } InputArgs;

typedef enum { FOCUS_CONSOLE_END = 0, FOCUS_CONSOLE_START, FOCUS_CONSOLE_TOGGLE } FocusConsole;

#define SHOW_STOPSCAN_TIP (true)
#define NO_TIP (false)

#define MAX_OPTIONS (9)
typedef struct {
    const char* item_string;
    const char* options_menu[MAX_OPTIONS];
    int num_options_menu;
    const char* actual_commands[MAX_OPTIONS];
    InputArgs needs_keyboard;
    FocusConsole focus_console;
    bool show_stopscan_tip;
} WifiMarauderItem;

// NUM_MENU_ITEMS defined in wifi_marauder_app_i.h - if you add an entry here, increment it!
const WifiMarauderItem items[NUM_MENU_ITEMS] = {
    {"View Log from", {"start", "end"}, 2, {"", ""}, NO_ARGS, FOCUS_CONSOLE_TOGGLE, NO_TIP},
    {"Scan",
     {"ap", "station"},
     2,
     {"scanap", "scansta"},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     SHOW_STOPSCAN_TIP},
    {"SSID",
     {"add rand", "add name", "remove"},
     3,
     {"ssid -a -g", "ssid -a -n", "ssid -r"},
     INPUT_ARGS,
     FOCUS_CONSOLE_START,
     NO_TIP},
    {"List",
     {"ap", "ssid", "station"},
     3,
     {"list -a", "list -s", "list -c"},
     NO_ARGS,
     FOCUS_CONSOLE_START,
     NO_TIP},
    {"Select",
     {"ap", "ssid", "station"},
     3,
     {"select -a", "select -s", "select -c"},
     INPUT_ARGS,
     FOCUS_CONSOLE_END,
     NO_TIP},
    {"Clear List",
     {"ap", "ssid", "station"},
     3,
     {"clearlist -a", "clearlist -s", "clearlist -c"},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     NO_TIP},
    {"Attack",
     {"deauth", "probe", "rickroll"},
     3,
     {"attack -t deauth", "attack -t probe", "attack -t rickroll"},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     SHOW_STOPSCAN_TIP},
    {"Targeted Deauth",
     {"station", "manual"},
     2,
     {"attack -t deauth -c", "attack -t deauth -s"},
     TOGGLE_ARGS,
     FOCUS_CONSOLE_END,
     SHOW_STOPSCAN_TIP},
    {"Beacon Spam",
     {"ap list", "ssid list", "random"},
     3,
     {"attack -t beacon -a", "attack -t beacon -l", "attack -t beacon -r"},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     SHOW_STOPSCAN_TIP},
    {"Sniff",
     {"beacon", "deauth", "esp", "pmkid", "probe", "pwn", "raw", "bt", "skim"},
     9,
     {"sniffbeacon",
      "sniffdeauth",
      "sniffesp",
      "sniffpmkid",
      "sniffprobe",
      "sniffpwn",
      "sniffraw",
      "sniffbt",
      "sniffskim"},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     SHOW_STOPSCAN_TIP},
    {"Signal Monitor", {""}, 1, {"sigmon"}, NO_ARGS, FOCUS_CONSOLE_END, SHOW_STOPSCAN_TIP},
    {"Channel",
     {"get", "set"},
     2,
     {"channel", "channel -s"},
     TOGGLE_ARGS,
     FOCUS_CONSOLE_END,
     NO_TIP},
    {"Settings",
     {"display", "restore", "ForcePMKID", "ForceProbe", "SavePCAP", "EnableLED", "other"},
     7,
     {"settings",
      "settings -r",
      "settings -s ForcePMKID enable",
      "settings -s ForceProbe enable",
      "settings -s SavePCAP enable",
      "settings -s EnableLED enable",
      "settings -s"},
     TOGGLE_ARGS,
     FOCUS_CONSOLE_START,
     NO_TIP},
    {"Update", {"ota", "sd"}, 2, {"update -w", "update -s"}, NO_ARGS, FOCUS_CONSOLE_END, NO_TIP},
    {"Reboot", {""}, 1, {"reboot"}, NO_ARGS, FOCUS_CONSOLE_END, NO_TIP},
    {"Help", {""}, 1, {"help"}, NO_ARGS, FOCUS_CONSOLE_START, SHOW_STOPSCAN_TIP},
    {"Reflash ESP32 (WIP)", {""}, 1, {""}, NO_ARGS, FOCUS_CONSOLE_END, NO_TIP},
    {"Scripts", {""}, 1, {""}, NO_ARGS, FOCUS_CONSOLE_END, NO_TIP},
    {"Save to flipper sdcard", // keep as last entry or change logic in callback below
     {""},
     1,
     {""},
     NO_ARGS,
     FOCUS_CONSOLE_START,
     NO_TIP},
};

static void wifi_marauder_scene_start_var_list_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    WifiMarauderApp* app = context;

    furi_assert(index < NUM_MENU_ITEMS);
    const WifiMarauderItem* item = &items[index];

    const int selected_option_index = app->selected_option_index[index];
    furi_assert(selected_option_index < item->num_options_menu);
    app->selected_tx_string = item->actual_commands[selected_option_index];
    app->is_command = (1 <= index);
    app->is_custom_tx_string = false;
    app->selected_menu_index = index;
    app->focus_console_start = (item->focus_console == FOCUS_CONSOLE_TOGGLE) ?
                                   (selected_option_index == 0) :
                                   item->focus_console;
    app->show_stopscan_tip = item->show_stopscan_tip;

    // TODO cleanup
    if(index == NUM_MENU_ITEMS - 3) {
        // flasher
        app->is_command = false;
        app->flash_mode = true;
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventStartFlasher);
        return;
    }

    app->flash_mode = false;

    if(!app->is_command && selected_option_index == 0) {
        // View Log from start
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventStartLogViewer);
        return;
    }

    if(app->selected_tx_string &&
       strncmp("sniffpmkid", app->selected_tx_string, strlen("sniffpmkid")) == 0) {
        // sniffpmkid submenu
        view_dispatcher_send_custom_event(
            app->view_dispatcher, WifiMarauderEventStartSniffPmkidOptions);
        return;
    }

    // Select automation script
    if(index == NUM_MENU_ITEMS - 2) {
        view_dispatcher_send_custom_event(
            app->view_dispatcher, WifiMarauderEventStartScriptSelect);
        return;
    }

    if(index == NUM_MENU_ITEMS - 1) {
        // "Save to flipper sdcard" special case - start SettingsInit widget
        view_dispatcher_send_custom_event(
            app->view_dispatcher, WifiMarauderEventStartSettingsInit);
        return;
    }

    bool needs_keyboard = (item->needs_keyboard == TOGGLE_ARGS) ? (selected_option_index != 0) :
                                                                  item->needs_keyboard;
    if(needs_keyboard) {
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventStartKeyboard);
    } else {
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventStartConsole);
    }
}

static void wifi_marauder_scene_start_var_list_change_callback(VariableItem* item) {
    furi_assert(item);

    WifiMarauderApp* app = variable_item_get_context(item);
    furi_assert(app);

    const WifiMarauderItem* menu_item = &items[app->selected_menu_index];
    uint8_t item_index = variable_item_get_current_value_index(item);
    furi_assert(item_index < menu_item->num_options_menu);
    variable_item_set_current_value_text(item, menu_item->options_menu[item_index]);
    app->selected_option_index[app->selected_menu_index] = item_index;
}

void wifi_marauder_scene_start_on_enter(void* context) {
    WifiMarauderApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    variable_item_list_set_enter_callback(
        var_item_list, wifi_marauder_scene_start_var_list_enter_callback, app);

    VariableItem* item;
    for(int i = 0; i < NUM_MENU_ITEMS; ++i) {
        item = variable_item_list_add(
            var_item_list,
            items[i].item_string,
            items[i].num_options_menu,
            wifi_marauder_scene_start_var_list_change_callback,
            app);
        variable_item_set_current_value_index(item, app->selected_option_index[i]);
        variable_item_set_current_value_text(
            item, items[i].options_menu[app->selected_option_index[i]]);
    }

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewVarItemList);

    // Wait, if the user hasn't initialized sdcard settings, let's prompt them once (then come back here)
    if(app->need_to_prompt_settings_init) {
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneSettingsInit);
    }
}

bool wifi_marauder_scene_start_on_event(void* context, SceneManagerEvent event) {
    WifiMarauderApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == WifiMarauderEventStartKeyboard) {
            scene_manager_set_scene_state(
                app->scene_manager, WifiMarauderSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifiMarauderSceneTextInput);
        } else if(event.event == WifiMarauderEventStartConsole) {
            scene_manager_set_scene_state(
                app->scene_manager, WifiMarauderSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifiMarauderSceneConsoleOutput);
        } else if(event.event == WifiMarauderEventStartSettingsInit) {
            scene_manager_set_scene_state(
                app->scene_manager, WifiMarauderSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifiMarauderSceneSettingsInit);
        } else if(event.event == WifiMarauderEventStartLogViewer) {
            scene_manager_set_scene_state(
                app->scene_manager, WifiMarauderSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifiMarauderSceneLogViewer);
        } else if(event.event == WifiMarauderEventStartScriptSelect) {
            scene_manager_set_scene_state(
                app->scene_manager, WifiMarauderSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifiMarauderSceneScriptSelect);
        } else if(event.event == WifiMarauderEventStartSniffPmkidOptions) {
            scene_manager_set_scene_state(
                app->scene_manager, WifiMarauderSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifiMarauderSceneSniffPmkidOptions);
        } else if(event.event == WifiMarauderEventStartFlasher) {
            scene_manager_set_scene_state(
                app->scene_manager, WifiMarauderSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifiMarauderSceneFlasher);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        app->selected_menu_index = variable_item_list_get_selected_item_index(app->var_item_list);
        consumed = true;
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_stop(app->scene_manager);
        view_dispatcher_stop(app->view_dispatcher);
        consumed = true;
    }

    return consumed;
}

void wifi_marauder_scene_start_on_exit(void* context) {
    WifiMarauderApp* app = context;
    variable_item_list_reset(app->var_item_list);
}
