#include "../wifi_marauder_app_i.h"

enum ScriptSettingsOption {
    ScriptSettingsOptionRepeat,
    ScriptSettingsOptionSavePcap,
    ScriptSettingsOptionEnableLed
};

const char* option_values[3] = {"No", "Yes", "Default"};

static void wifi_marauder_scene_script_settings_enter_callback(void* context, uint32_t index) {
    WifiMarauderApp* app = context;
    // Accept script repeat value
    if(index == ScriptSettingsOptionRepeat) {
        scene_manager_set_scene_state(app->scene_manager, WifiMarauderSceneScriptSettings, index);
        app->user_input_type = WifiMarauderUserInputTypeNumber;
        app->user_input_number_reference = &app->script->repeat;
        scene_manager_next_scene(app->scene_manager, WifiMarauderSceneUserInput);
    }
}

static void wifi_marauder_scene_script_settings_change_callback(VariableItem* item) {
    WifiMarauderApp* app = variable_item_get_context(item);

    uint8_t current_option = variable_item_list_get_selected_item_index(app->var_item_list);
    uint8_t option_value_index = variable_item_get_current_value_index(item);

    switch(current_option) {
    case ScriptSettingsOptionSavePcap:
        variable_item_set_current_value_text(item, option_values[option_value_index]);
        app->script->save_pcap = option_value_index;
        break;
    case ScriptSettingsOptionEnableLed:
        variable_item_set_current_value_text(item, option_values[option_value_index]);
        app->script->enable_led = option_value_index;
        break;
    }
}

void wifi_marauder_scene_script_settings_on_enter(void* context) {
    WifiMarauderApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;
    variable_item_list_set_enter_callback(
        app->var_item_list, wifi_marauder_scene_script_settings_enter_callback, app);

    // Script repeat option
    VariableItem* repeat_item = variable_item_list_add(app->var_item_list, "Repeat", 1, NULL, app);
    char repeat_str[32];
    snprintf(repeat_str, sizeof(repeat_str), "%d", app->script->repeat);
    variable_item_set_current_value_text(repeat_item, repeat_str);

    // Save PCAP option
    VariableItem* save_pcap_item = variable_item_list_add(
        app->var_item_list,
        "Save PCAP",
        3,
        wifi_marauder_scene_script_settings_change_callback,
        app);
    variable_item_set_current_value_index(save_pcap_item, app->script->save_pcap);
    variable_item_set_current_value_text(save_pcap_item, option_values[app->script->save_pcap]);

    // Enable board LED option
    VariableItem* enable_led_item = variable_item_list_add(
        app->var_item_list,
        "Enable LED",
        3,
        wifi_marauder_scene_script_settings_change_callback,
        app);
    variable_item_set_current_value_index(enable_led_item, app->script->enable_led);
    variable_item_set_current_value_text(enable_led_item, option_values[app->script->enable_led]);

    variable_item_list_set_selected_item(
        var_item_list,
        scene_manager_get_scene_state(app->scene_manager, WifiMarauderSceneScriptSettings));
    view_dispatcher_switch_to_view(app->view_dispatcher, WifiMarauderAppViewVarItemList);
}

bool wifi_marauder_scene_script_settings_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void wifi_marauder_scene_script_settings_on_exit(void* context) {
    WifiMarauderApp* app = context;
    variable_item_list_reset(app->var_item_list);
}
