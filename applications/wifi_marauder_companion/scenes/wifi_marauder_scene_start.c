#include "../wifi_marauder_app_i.h"

#define NUM_MENU_ITEMS (29)

// For each command, define whether additional arguments are needed
// (enabling text input to fill them out), and whether the console
// text box should focus at the start of the output or the end
#define INPUT_ARGS          (true)
#define NO_ARGS             (false)
#define FOCUS_CONSOLE_START (true)
#define FOCUS_CONSOLE_END   (false)
struct WifiMarauderItem {
    const char* item_string;
    bool needs_keyboard;
    bool focus_console_start;
};

const struct WifiMarauderItem items[NUM_MENU_ITEMS] = {
    { "View Log (start)", NO_ARGS, FOCUS_CONSOLE_START },
    { "View Log (end)", NO_ARGS, FOCUS_CONSOLE_END },
    { "attack -t beacon -l", NO_ARGS, FOCUS_CONSOLE_END },
    { "attack -t beacon -r", NO_ARGS, FOCUS_CONSOLE_END },
    { "attack -t beacon -a", NO_ARGS, FOCUS_CONSOLE_END },
    { "attack -t deauth", NO_ARGS, FOCUS_CONSOLE_END },
    { "attack -t probe", NO_ARGS, FOCUS_CONSOLE_END },
    { "attack -t rickroll", NO_ARGS, FOCUS_CONSOLE_END },
    { "channel", NO_ARGS, FOCUS_CONSOLE_END },
    { "channel -s", INPUT_ARGS, FOCUS_CONSOLE_END },
    { "clearlist -a", NO_ARGS, FOCUS_CONSOLE_END },
    { "clearlist -s", NO_ARGS, FOCUS_CONSOLE_END },
    { "help", NO_ARGS, FOCUS_CONSOLE_START },
    { "list -a", NO_ARGS, FOCUS_CONSOLE_START },
    { "list -s", NO_ARGS, FOCUS_CONSOLE_START },
    { "reboot", NO_ARGS, FOCUS_CONSOLE_END },
    { "scanap", NO_ARGS, FOCUS_CONSOLE_END },
    { "select -a", INPUT_ARGS, FOCUS_CONSOLE_END },
    { "select -s", INPUT_ARGS, FOCUS_CONSOLE_END },
    { "sniffbeacon", NO_ARGS, FOCUS_CONSOLE_END },
    { "sniffdeauth", NO_ARGS, FOCUS_CONSOLE_END },
    { "sniffesp", NO_ARGS, FOCUS_CONSOLE_END },
    { "sniffpmkid", NO_ARGS, FOCUS_CONSOLE_END },
    { "sniffpmkid -c", INPUT_ARGS, FOCUS_CONSOLE_END },
    { "sniffpwn", NO_ARGS, FOCUS_CONSOLE_END },
    { "ssid -a -g", INPUT_ARGS, FOCUS_CONSOLE_END },
    { "ssid -a -n", INPUT_ARGS, FOCUS_CONSOLE_END },
    { "ssid -r", INPUT_ARGS, FOCUS_CONSOLE_END },
    { "update -w", NO_ARGS, FOCUS_CONSOLE_END },
};

static void wifi_marauder_scene_start_var_list_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    WifiMarauderApp* app = context;
    app->selected_tx_string = items[index].item_string;
    app->is_command = (2 <= index);
    app->is_custom_tx_string = false;
    app->selected_menu_index = index;
    app->focus_console_start = items[index].focus_console_start;
    if (items[index].needs_keyboard) {
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventStartKeyboard);
    } else {
        view_dispatcher_send_custom_event(app->view_dispatcher, WifiMarauderEventStartConsole);
    }
}

void wifi_marauder_scene_start_on_enter(void* context) {
    WifiMarauderApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    variable_item_list_set_enter_callback(
        var_item_list, wifi_marauder_scene_start_var_list_enter_callback, app);

    // TODO: organize menu
    for (int i = 0; i < NUM_MENU_ITEMS; ++i) {
        variable_item_list_add(var_item_list, items[i].item_string, 0, NULL, NULL);
    }

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewVarItemList);
}

bool wifi_marauder_scene_start_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    WifiMarauderApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if (event.event == WifiMarauderEventStartKeyboard) {
            scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifiMarauderAppViewTextInput);
        } else if (event.event == WifiMarauderEventStartConsole) {
            scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneStart, app->selected_menu_index);
            scene_manager_next_scene(app->scene_manager, WifiMarauderAppViewConsoleOutput);
        }
        consumed = true;
    }

    return consumed;
}

void wifi_marauder_scene_start_on_exit(void* context) {
    WifiMarauderApp* app = context;
    variable_item_list_reset(app->var_item_list);
}
