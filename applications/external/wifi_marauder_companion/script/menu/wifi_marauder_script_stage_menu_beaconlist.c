#include "../../wifi_marauder_app_i.h"

void wifi_marauder_beaconlist_stage_ssids_select_callback(void* context) {
    WifiMarauderApp* app = context;
    WifiMarauderScriptStageBeaconList* stage_beaconlist = app->script_edit_selected_stage->stage;
    app->script_stage_edit_strings_reference = &stage_beaconlist->ssids;
    app->script_stage_edit_string_count_reference = &stage_beaconlist->ssid_count;
}

void wifi_marauder_beaconlist_stage_random_ssids_setup_callback(VariableItem* item) {
    WifiMarauderApp* app = variable_item_get_context(item);
    WifiMarauderScriptStageBeaconList* stage = app->script_edit_selected_stage->stage;
    char random_ssids_str[32];
    snprintf(random_ssids_str, sizeof(random_ssids_str), "%d", stage->random_ssids);
    variable_item_set_current_value_text(item, random_ssids_str);
}

void wifi_marauder_beaconlist_stage_random_ssids_select_callback(void* context) {
    WifiMarauderApp* app = context;
    WifiMarauderScriptStageBeaconList* stage_beaconlist = app->script_edit_selected_stage->stage;
    app->user_input_number_reference = &stage_beaconlist->random_ssids;
}

void wifi_marauder_beaconlist_stage_timeout_setup_callback(VariableItem* item) {
    WifiMarauderApp* app = variable_item_get_context(item);
    WifiMarauderScriptStageBeaconList* stage = app->script_edit_selected_stage->stage;
    char timeout_str[32];
    snprintf(timeout_str, sizeof(timeout_str), "%d", stage->timeout);
    variable_item_set_current_value_text(item, timeout_str);
}

void wifi_marauder_beaconlist_stage_timeout_select_callback(void* context) {
    WifiMarauderApp* app = context;
    WifiMarauderScriptStageBeaconList* stage_beaconlist = app->script_edit_selected_stage->stage;
    app->user_input_number_reference = &stage_beaconlist->timeout;
}

void wifi_marauder_script_stage_menu_beaconlist_load(WifiMarauderScriptStageMenu* stage_menu) {
    stage_menu->num_items = 3;
    stage_menu->items = malloc(3 * sizeof(WifiMarauderScriptMenuItem));

    stage_menu->items[0] = (WifiMarauderScriptMenuItem){
        .name = strdup("SSIDs"),
        .type = WifiMarauderScriptMenuItemTypeListString,
        .num_options = 1,
        .select_callback = wifi_marauder_beaconlist_stage_ssids_select_callback};
    stage_menu->items[1] = (WifiMarauderScriptMenuItem){
        .name = strdup("Generate random"),
        .type = WifiMarauderScriptMenuItemTypeNumber,
        .num_options = 1,
        .setup_callback = wifi_marauder_beaconlist_stage_random_ssids_setup_callback,
        .select_callback = wifi_marauder_beaconlist_stage_random_ssids_select_callback};
    stage_menu->items[2] = (WifiMarauderScriptMenuItem){
        .name = strdup("Timeout"),
        .type = WifiMarauderScriptMenuItemTypeNumber,
        .num_options = 1,
        .setup_callback = wifi_marauder_beaconlist_stage_timeout_setup_callback,
        .select_callback = wifi_marauder_beaconlist_stage_timeout_select_callback};
}