#include "../infrared_i.h"
#include <furi_hal_infrared.h>

uint8_t value_index_ir;

#define DEB_PINS_COUNT (sizeof(infrared_debug_cfg_variables_text) / sizeof(char* const))
const char* const infrared_debug_cfg_variables_text[] = {
    "Internal",
    "2 (A7)",
};

static void infrared_scene_debug_settings_changed(VariableItem* item) {
    Infrared* infrared = variable_item_get_context(item);
    value_index_ir = variable_item_get_current_value_index(item);
    UNUSED(infrared);

    variable_item_set_current_value_text(item, infrared_debug_cfg_variables_text[value_index_ir]);

    furi_hal_infrared_set_debug_out(value_index_ir);
}
static void infrared_debug_settings_start_var_list_enter_callback(void* context, uint32_t index) {
    Infrared* infrared = context;
    view_dispatcher_send_custom_event(infrared->view_dispatcher, index);
}

void infrared_scene_debug_settings_on_enter(void* context) {
    Infrared* infrared = context;

    VariableItemList* variable_item_list = infrared->variable_item_list;

    value_index_ir = furi_hal_infrared_get_debug_out_status();
    VariableItem* item = variable_item_list_add(
        variable_item_list,
        "Send signal to",
        DEB_PINS_COUNT,
        infrared_scene_debug_settings_changed,
        infrared);

    variable_item_list_set_enter_callback(
        variable_item_list, infrared_debug_settings_start_var_list_enter_callback, infrared);

    variable_item_set_current_value_index(item, value_index_ir);
    variable_item_set_current_value_text(item, infrared_debug_cfg_variables_text[value_index_ir]);

    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewVariableItemList);
}

bool infrared_scene_debug_settings_on_event(void* context, SceneManagerEvent event) {
    Infrared* infrared = context;
    UNUSED(infrared);
    UNUSED(event);

    return false;
}

void infrared_scene_debug_settings_on_exit(void* context) {
    Infrared* infrared = context;
    variable_item_list_reset(infrared->variable_item_list);
}
