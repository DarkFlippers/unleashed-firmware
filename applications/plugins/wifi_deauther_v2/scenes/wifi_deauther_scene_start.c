#include "../wifi_deauther_app_i.h"

// For each command, define whether additional arguments are needed
// (enabling text input to fill them out), and whether the console
// text box should focus at the start of the output or the end
typedef enum { NO_ARGS = 0, INPUT_ARGS, TOGGLE_ARGS } InputArgs;

typedef enum { FOCUS_CONSOLE_END = 0, FOCUS_CONSOLE_START, FOCUS_CONSOLE_TOGGLE } FocusConsole;

#define SHOW_STOPSCAN_TIP (true)
#define NO_TIP (false)

#define MAX_OPTIONS (6)
typedef struct {
    const char* item_string;
    const char* options_menu[MAX_OPTIONS];
    int num_options_menu;
    const char* actual_commands[MAX_OPTIONS];
    InputArgs needs_keyboard;
    FocusConsole focus_console;
    bool show_stopscan_tip;
} WifideautherItem;

// NUM_MENU_ITEMS defined in wifi_deauther_app_i.h - if you add an entry here, increment it!
const WifideautherItem MenuItems[NUM_MENU_ITEMS] = {
    {"View Log from", {"start", "end"}, 2, {}, NO_ARGS, FOCUS_CONSOLE_TOGGLE, NO_TIP},
    {"Help", {""}, 1, {"help"}, NO_ARGS, FOCUS_CONSOLE_END, NO_TIP},
    {"Stop", {""}, 1, {"stop all"}, NO_ARGS, FOCUS_CONSOLE_END, NO_TIP},
    {"Scan",
     {"All", "SSIDs", "Stations"},
     3,
     {"scan", "scan aps", "scan stations"},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     NO_TIP},
    {"Select",
     {"All", "SSIDs", "Stations"},
     3,
     {"select all", "select aps", "select stations"},
     INPUT_ARGS,
     FOCUS_CONSOLE_END,
     NO_TIP},
    {"Deselect",
     {"All", "SSIDs", "Stations"},
     3,
     {"deselect all", "deselect aps", "deselect stations"},
     INPUT_ARGS,
     FOCUS_CONSOLE_END,
     NO_TIP},
    {"Show",
     {"SSIDs", "Stations", "All", "Selected"},
     4,
     {"show ap", "show station", "show all", "show selected"},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     NO_TIP},
    {"Attack",
     {"deauth", "deauthall", "beacon", "probe"},
     4,
     {"attack deauth", "attack deauthall", "attack beacon", "attack probe"},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     SHOW_STOPSCAN_TIP},
    {"Settings",
     {"Get", "Remove AP", "Set SSID", "Set Pass", "Save"},
     5,
     {"get settings",
      "set webinterface false",
      "set ssid: pwned",
      "set password: deauther",
      "save settings"},
     INPUT_ARGS,
     FOCUS_CONSOLE_END,
     NO_TIP},
    {"Sysinfo", {""}, 1, {"sysinfo"}, NO_ARGS, FOCUS_CONSOLE_END, NO_TIP},
    {"Reboot", {""}, 1, {"reboot"}, NO_ARGS, FOCUS_CONSOLE_END, NO_TIP},
};

static void wifi_deauther_scene_start_var_list_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    WifideautherApp* app = context;
    if(app->selected_option_index[index] < MenuItems[index].num_options_menu) {
        app->selected_tx_string =
            MenuItems[index].actual_commands[app->selected_option_index[index]];
    }
    app->is_command = (1 <= index);
    app->is_custom_tx_string = false;
    app->selected_menu_index = index;
    app->focus_console_start = (MenuItems[index].focus_console == FOCUS_CONSOLE_TOGGLE) ?
                                   (app->selected_option_index[index] == 0) :
                                   MenuItems[index].focus_console;
    app->show_stopscan_tip = MenuItems[index].show_stopscan_tip;

    bool needs_keyboard = (MenuItems[index].needs_keyboard == TOGGLE_ARGS) ?
                              (app->selected_option_index[index] != 0) :
                              MenuItems[index].needs_keyboard;
    if(needs_keyboard) {
        view_dispatcher_send_custom_event(app->view_dispatcher, WifideautherEventStartKeyboard);
    } else {
        view_dispatcher_send_custom_event(app->view_dispatcher, WifideautherEventStartConsole);
    }
}

static void wifi_deauther_scene_start_var_list_change_callback(VariableItem* item) {
    furi_assert(item);

    WifideautherApp* app = variable_item_get_context(item);
    furi_assert(app);

    const WifideautherItem* menu_item = &MenuItems[app->selected_menu_index];
    uint8_t item_index = variable_item_get_current_value_index(item);
    furi_assert(item_index < menu_item->num_options_menu);
    variable_item_set_current_value_text(item, menu_item->options_menu[item_index]);
    app->selected_option_index[app->selected_menu_index] = item_index;
}

void wifi_deauther_scene_start_on_enter(void* context) {
    WifideautherApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    variable_item_list_set_enter_callback(
        var_item_list, wifi_deauther_scene_start_var_list_enter_callback, app);

    VariableItem* item;
    for(int i = 0; i < NUM_MENU_ITEMS; ++i) {
        item = variable_item_list_add(
            var_item_list,
            MenuItems[i].item_string,
            MenuItems[i].num_options_menu,
            wifi_deauther_scene_start_var_list_change_callback,
            app);
        if(MenuItems[i].num_options_menu) {
            variable_item_set_current_value_index(item, app->selected_option_index[i]);
            variable_item_set_current_value_text(
                item, MenuItems[i].options_menu[app->selected_option_index[i]]);
        }
    }

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, WifideautherSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, WifideautherAppViewVarItemList);
}

bool wifi_deauther_scene_start_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    WifideautherApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == WifideautherEventStartKeyboard) {
            scene_manager_set_scene_state(
                app->scene_manager, WifideautherSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifideautherAppViewTextInput);
        } else if(event.event == WifideautherEventStartConsole) {
            scene_manager_set_scene_state(
                app->scene_manager, WifideautherSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifideautherAppViewConsoleOutput);
        }
        consumed = true;
    } else if(event.type == SceneManagerEventTypeTick) {
        app->selected_menu_index = variable_item_list_get_selected_item_index(app->var_item_list);
        consumed = true;
    }

    return consumed;
}

void wifi_deauther_scene_start_on_exit(void* context) {
    WifideautherApp* app = context;
    variable_item_list_reset(app->var_item_list);
}
