#include "../subghz_i.h"
#include "../helpers/subghz_custom_event.h"
#include <lib/toolbox/value_index.h>
#include <applications/drivers/subghz/cc1101_ext/cc1101_ext_interconnect.h>

#define RADIO_DEVICE_COUNT 2
const char* const radio_device_text[RADIO_DEVICE_COUNT] = {
    "Internal",
    "External",
};

const uint32_t radio_device_value[RADIO_DEVICE_COUNT] = {
    SubGhzRadioDeviceTypeInternal,
    SubGhzRadioDeviceTypeExternalCC1101,
};

#define TIMESTAMP_NAMES_COUNT 2
const char* const timestamp_names_text[TIMESTAMP_NAMES_COUNT] = {
    "OFF",
    "ON",
};

#define DEBUG_P_COUNT 2
const char* const debug_pin_text[DEBUG_P_COUNT] = {
    "OFF",
    "17(1W)",
};

#define DEBUG_COUNTER_COUNT 6
const char* const debug_counter_text[DEBUG_COUNTER_COUNT] = {
    "+1",
    "+2",
    "+3",
    "+4",
    "+5",
    "+10",
};

static void subghz_scene_radio_settings_set_device(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    if(!subghz_txrx_radio_device_is_external_connected(
           subghz->txrx, SUBGHZ_DEVICE_CC1101_EXT_NAME) &&
       radio_device_value[index] == SubGhzRadioDeviceTypeExternalCC1101) {
        //ToDo correct if there is more than 1 module
        index = 0;
    }
    variable_item_set_current_value_text(item, radio_device_text[index]);
    subghz_txrx_radio_device_set(subghz->txrx, radio_device_value[index]);
}

static void subghz_scene_receiver_config_set_debug_pin(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, debug_pin_text[index]);

    subghz_txrx_set_debug_pin_state(subghz->txrx, index == 1);
}

static void subghz_scene_receiver_config_set_debug_counter(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, debug_counter_text[index]);

    switch(index) {
    case 0:
        furi_hal_subghz_set_rolling_counter_mult(1);
        break;
    case 1:
        furi_hal_subghz_set_rolling_counter_mult(2);
        break;
    case 2:
        furi_hal_subghz_set_rolling_counter_mult(3);
        break;
    case 3:
        furi_hal_subghz_set_rolling_counter_mult(4);
        break;
    case 4:
        furi_hal_subghz_set_rolling_counter_mult(5);
        break;
    case 5:
        furi_hal_subghz_set_rolling_counter_mult(10);
        break;
    default:
        break;
    }
}

// static void subghz_scene_receiver_config_set_ext_mod_power(VariableItem* item) {
//     SubGhz* subghz = variable_item_get_context(item);
//     uint8_t index = variable_item_get_current_value_index(item);

//     variable_item_set_current_value_text(item, ext_mod_power_text[index]);

//     furi_hal_subghz_set_external_power_disable(index == 1);
//     if(index == 1) {
//         furi_hal_subghz_disable_ext_power();
//     } else {
//         furi_hal_subghz_enable_ext_power();
//     }

//     subghz->last_settings->external_module_power_5v_disable = index == 1;
//     subghz_last_settings_save(subghz->last_settings);
// }

static void subghz_scene_receiver_config_set_timestamp_file_names(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, timestamp_names_text[index]);

    subghz->last_settings->timestamp_file_names = (index == 1);
    subghz_last_settings_save(subghz->last_settings);
}

void subghz_scene_radio_settings_on_enter(void* context) {
    SubGhz* subghz = context;

    VariableItemList* variable_item_list = subghz->variable_item_list;
    uint8_t value_index;
    VariableItem* item;

    uint8_t value_count_device = RADIO_DEVICE_COUNT;
    if(subghz_txrx_radio_device_get(subghz->txrx) == SubGhzRadioDeviceTypeInternal &&
       !subghz_txrx_radio_device_is_external_connected(subghz->txrx, SUBGHZ_DEVICE_CC1101_EXT_NAME))
        value_count_device = 1; // Only 1 item if external disconnected
    item = variable_item_list_add(
        subghz->variable_item_list,
        "Module",
        value_count_device,
        subghz_scene_radio_settings_set_device,
        subghz);
    value_index = value_index_uint32(
        subghz_txrx_radio_device_get(subghz->txrx), radio_device_value, value_count_device);
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, radio_device_text[value_index]);

    item = variable_item_list_add(
        variable_item_list,
        "Time in names",
        TIMESTAMP_NAMES_COUNT,
        subghz_scene_receiver_config_set_timestamp_file_names,
        subghz);
    value_index = subghz->last_settings->timestamp_file_names;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, timestamp_names_text[value_index]);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        item = variable_item_list_add(
            variable_item_list,
            "Counter incr.",
            DEBUG_COUNTER_COUNT,
            subghz_scene_receiver_config_set_debug_counter,
            subghz);
        switch(furi_hal_subghz_get_rolling_counter_mult()) {
        case 1:
            value_index = 0;
            break;
        case 2:
            value_index = 1;
            break;
        case 3:
            value_index = 2;
            break;
        case 4:
            value_index = 3;
            break;
        case 5:
            value_index = 4;
            break;
        case 10:
            value_index = 5;
            break;
        default:
            break;
        }
    } else {
        item = variable_item_list_add(
            variable_item_list,
            "Counter incr.",
            3,
            subghz_scene_receiver_config_set_debug_counter,
            subghz);
        switch(furi_hal_subghz_get_rolling_counter_mult()) {
        case 1:
            value_index = 0;
            break;
        case 2:
            value_index = 1;
            break;
        case 3:
            value_index = 2;
            break;
        default:
            // Reset to default value
            value_index = 0;
            furi_hal_subghz_set_rolling_counter_mult(1);
            break;
        }
    }
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, debug_counter_text[value_index]);

    if(furi_hal_rtc_is_flag_set(FuriHalRtcFlagDebug)) {
        item = variable_item_list_add(
            variable_item_list,
            "Debug Pin",
            DEBUG_P_COUNT,
            subghz_scene_receiver_config_set_debug_pin,
            subghz);
        value_index = subghz_txrx_get_debug_pin_state(subghz->txrx);
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(item, debug_pin_text[value_index]);
    }

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdVariableItemList);
}

bool subghz_scene_radio_settings_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    UNUSED(subghz);
    UNUSED(event);

    return false;
}

void subghz_scene_radio_settings_on_exit(void* context) {
    SubGhz* subghz = context;
    variable_item_list_set_selected_item(subghz->variable_item_list, 0);
    variable_item_list_reset(subghz->variable_item_list);
}
