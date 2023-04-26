#include "../../wifi_marauder_app_i.h"

void wifi_marauder_select_stage_type_setup_callback(VariableItem* item) {
    WifiMarauderApp* app = variable_item_get_context(item);
    WifiMarauderScriptStageSelect* stage = app->script_edit_selected_stage->stage;
    variable_item_set_current_value_index(item, stage->type);
}

void wifi_marauder_select_stage_type_change_callback(VariableItem* item) {
    WifiMarauderApp* app = variable_item_get_context(item);

    // Get menu item
    uint8_t current_stage_index = variable_item_list_get_selected_item_index(app->var_item_list);
    const WifiMarauderScriptMenuItem* menu_item =
        &app->script_stage_menu->items[current_stage_index];

    // Defines the text of the selected option
    uint8_t option_index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, menu_item->options[option_index]);

    // Updates the attribute value of the current stage
    WifiMarauderScriptStageSelect* stage = app->script_edit_selected_stage->stage;
    stage->type = option_index;
}

void wifi_marauder_select_stage_filter_setup_callback(VariableItem* item) {
    WifiMarauderApp* app = variable_item_get_context(item);
    WifiMarauderScriptStageSelect* stage = app->script_edit_selected_stage->stage;

    if(stage->filter != NULL) {
        variable_item_set_current_value_index(item, 0);
        variable_item_set_current_value_text(item, stage->filter);
    } else {
        variable_item_set_current_value_index(item, 1);
    }
}

void wifi_marauder_select_stage_filter_change_callback(VariableItem* item) {
    WifiMarauderApp* app = variable_item_get_context(item);
    WifiMarauderScriptStageSelect* stage = app->script_edit_selected_stage->stage;

    // Clears the filter if you change the option. Flipper input box does not accept blank text
    if(variable_item_get_current_value_index(item) == 1) {
        stage->filter = NULL;
        variable_item_set_current_value_index(item, 0);
        variable_item_set_values_count(item, 1);
    }

    if(stage->filter != NULL) {
        variable_item_set_current_value_text(item, stage->filter);
    } else {
        variable_item_set_current_value_text(item, "");
    }
}

void wifi_marauder_select_stage_filter_select_callback(void* context) {
    WifiMarauderApp* app = context;
    WifiMarauderScriptStageSelect* stage_select = app->script_edit_selected_stage->stage;
    if(stage_select->filter == NULL) {
        stage_select->filter = malloc(128);
    }
    app->user_input_string_reference = &stage_select->filter;
}

void wifi_marauder_select_stage_indexes_select_callback(void* context) {
    WifiMarauderApp* app = context;
    WifiMarauderScriptStageSelect* stage_select = app->script_edit_selected_stage->stage;
    app->script_stage_edit_numbers_reference = &stage_select->indexes;
    app->script_stage_edit_number_count_reference = &stage_select->index_count;
}

void wifi_marauder_script_stage_menu_select_load(WifiMarauderScriptStageMenu* stage_menu) {
    stage_menu->num_items = 3;
    stage_menu->items = malloc(3 * sizeof(WifiMarauderScriptMenuItem));

    stage_menu->items[0] = (WifiMarauderScriptMenuItem){
        .name = strdup("Type"),
        .type = WifiMarauderScriptMenuItemTypeOptionsString,
        .num_options = 2,
        .options = {"ap", "station"},
        .setup_callback = wifi_marauder_select_stage_type_setup_callback,
        .change_callback = wifi_marauder_select_stage_type_change_callback};
    stage_menu->items[1] = (WifiMarauderScriptMenuItem){
        .name = strdup("Filter"),
        .type = WifiMarauderScriptMenuItemTypeString,
        .num_options = 2,
        .setup_callback = wifi_marauder_select_stage_filter_setup_callback,
        .change_callback = wifi_marauder_select_stage_filter_change_callback,
        .select_callback = wifi_marauder_select_stage_filter_select_callback};
    stage_menu->items[2] = (WifiMarauderScriptMenuItem){
        .name = strdup("Indexes"),
        .type = WifiMarauderScriptMenuItemTypeListNumber,
        .num_options = 1,
        .select_callback = wifi_marauder_select_stage_indexes_select_callback};
}