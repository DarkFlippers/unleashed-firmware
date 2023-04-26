#include "../../wifi_marauder_app_i.h"

void wifi_marauder_exec_stage_filter_setup_callback(VariableItem* item) {
    WifiMarauderApp* app = variable_item_get_context(item);
    WifiMarauderScriptStageExec* stage = app->script_edit_selected_stage->stage;
    if(stage->command != NULL) {
        variable_item_set_current_value_text(item, stage->command);
    }
}

void wifi_marauder_exec_stage_filter_select_callback(void* context) {
    WifiMarauderApp* app = context;
    WifiMarauderScriptStageExec* stage_select = app->script_edit_selected_stage->stage;
    if(stage_select->command == NULL) {
        stage_select->command = malloc(128);
    }
    app->user_input_string_reference = &stage_select->command;
}

void wifi_marauder_script_stage_menu_exec_load(WifiMarauderScriptStageMenu* stage_menu) {
    stage_menu->num_items = 1;
    stage_menu->items = malloc(1 * sizeof(WifiMarauderScriptMenuItem));

    stage_menu->items[0] = (WifiMarauderScriptMenuItem){
        .name = strdup("Command"),
        .type = WifiMarauderScriptMenuItemTypeString,
        .num_options = 1,
        .setup_callback = wifi_marauder_exec_stage_filter_setup_callback,
        .select_callback = wifi_marauder_exec_stage_filter_select_callback};
}