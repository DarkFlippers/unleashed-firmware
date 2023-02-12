#include "../subghz_i.h"
#include "../helpers/subghz_custom_event.h"

uint8_t value_index;
uint8_t value_index2;

#define EXT_MODULES_COUNT (sizeof(radio_modules_variables_text) / sizeof(char* const))
const char* const radio_modules_variables_text[] = {
    "Internal",
    "External",
};

#define DEBUG_P_COUNT 2
const char* const debug_pin_text[DEBUG_P_COUNT] = {
    "OFF",
    "17(1W)",
};

static void subghz_scene_ext_module_changed(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    value_index = variable_item_get_current_value_index(item);
    UNUSED(subghz);

    variable_item_set_current_value_text(item, radio_modules_variables_text[value_index]);
}
static void subghz_ext_module_start_var_list_enter_callback(void* context, uint32_t index) {
    SubGhz* subghz = context;
    view_dispatcher_send_custom_event(subghz->view_dispatcher, index);
}

static void subghz_scene_receiver_config_set_debug_pin(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, debug_pin_text[index]);

    subghz->txrx->debug_pin_state = index == 1;
}

void subghz_scene_ext_module_settings_on_enter(void* context) {
    SubGhz* subghz = context;

    VariableItemList* variable_item_list = subghz->variable_item_list;

    value_index = furi_hal_subghz.radio_type;
    VariableItem* item = variable_item_list_add(
        variable_item_list, "Module", EXT_MODULES_COUNT, subghz_scene_ext_module_changed, subghz);

    variable_item_list_set_enter_callback(
        variable_item_list, subghz_ext_module_start_var_list_enter_callback, subghz);

    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, radio_modules_variables_text[value_index]);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        item = variable_item_list_add(
            subghz->variable_item_list,
            "Debug Pin:",
            DEBUG_P_COUNT,
            subghz_scene_receiver_config_set_debug_pin,
            subghz);
        value_index2 = subghz->txrx->debug_pin_state;
        variable_item_set_current_value_index(item, value_index2);
        variable_item_set_current_value_text(item, debug_pin_text[value_index2]);
    }

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdVariableItemList);
}

bool subghz_scene_ext_module_settings_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    UNUSED(subghz);
    UNUSED(event);

    // Set selected radio module
    furi_hal_subghz_set_radio_type(value_index);

    // Check if module is present, if no -> show error
    if(!furi_hal_subghz_check_radio()) {
        value_index = 0;
        furi_hal_subghz_set_radio_type(value_index);
        furi_string_set(subghz->error_str, "Please connect\nexternal radio");
        scene_manager_next_scene(subghz->scene_manager, SubGhzSceneShowErrorSub);
    }

    return false;
}

void subghz_scene_ext_module_settings_on_exit(void* context) {
    SubGhz* subghz = context;
    variable_item_list_reset(subghz->variable_item_list);
}
