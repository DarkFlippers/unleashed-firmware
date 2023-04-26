#include "../../wifi_marauder_app_i.h"

void wifi_marauder_deauth_stage_timeout_setup_callback(VariableItem* item) {
    WifiMarauderApp* app = variable_item_get_context(item);
    WifiMarauderScriptStageDeauth* stage = app->script_edit_selected_stage->stage;
    char timeout_str[32];
    snprintf(timeout_str, sizeof(timeout_str), "%d", stage->timeout);
    variable_item_set_current_value_text(item, timeout_str);
}

void wifi_marauder_deauth_stage_timeout_select_callback(void* context) {
    WifiMarauderApp* app = context;
    WifiMarauderScriptStageDeauth* stage_deauth = app->script_edit_selected_stage->stage;
    app->user_input_number_reference = &stage_deauth->timeout;
}

void wifi_marauder_script_stage_menu_deauth_load(WifiMarauderScriptStageMenu* stage_menu) {
    stage_menu->num_items = 1;
    stage_menu->items = malloc(1 * sizeof(WifiMarauderScriptMenuItem));

    stage_menu->items[0] = (WifiMarauderScriptMenuItem){
        .name = strdup("Timeout"),
        .type = WifiMarauderScriptMenuItemTypeNumber,
        .num_options = 1,
        .setup_callback = wifi_marauder_deauth_stage_timeout_setup_callback,
        .select_callback = wifi_marauder_deauth_stage_timeout_select_callback};
}