#include "../subghz_i.h" // IWYU pragma: keep
#include <lib/toolbox/value_index.h>
#include <applications/drivers/subghz/cc1101_ext/cc1101_ext_interconnect.h>
#include <lib/subghz/devices/cc1101_int/cc1101_int_interconnect.h>

enum SubGhzRadioSettingIndex {
    SubGhzRadioSettingIndexDevice,
};

#define RADIO_DEVICE_COUNT 2
const char* const radio_device_text[RADIO_DEVICE_COUNT] = {
    "Internal",
    "External",
};

const uint32_t radio_device_value[RADIO_DEVICE_COUNT] = {
    SubGhzRadioDeviceTypeInternal,
    SubGhzRadioDeviceTypeExternalCC1101,
};

const char* const radio_device_name[RADIO_DEVICE_COUNT] = {
    SUBGHZ_DEVICE_CC1101_INT_NAME,
    SUBGHZ_DEVICE_CC1101_EXT_NAME,
};

static uint8_t subghz_scene_radio_settings_next_index_connect_ext_device(
    SubGhz* subghz,
    uint8_t current_index) {
    uint8_t index = 0;
    for(index = current_index; index < RADIO_DEVICE_COUNT; index++) {
        if(subghz_txrx_radio_device_is_external_connected(subghz->txrx, radio_device_name[index])) {
            break;
        }
    }
    if(index == RADIO_DEVICE_COUNT) index = 0;
    return index;
}

static void subghz_scene_radio_settings_set_device(VariableItem* item) {
    SubGhz* subghz = variable_item_get_context(item);
    uint8_t current_index = variable_item_get_current_value_index(item);
    uint8_t index =
        subghz_scene_radio_settings_next_index_connect_ext_device(subghz, current_index);
    variable_item_set_current_value_text(item, radio_device_text[index]);
    subghz_txrx_radio_device_set(subghz->txrx, radio_device_value[index]);
}

void subghz_scene_radio_settings_on_enter(void* context) {
    SubGhz* subghz = context;
    VariableItem* item;
    uint8_t value_index;

    uint8_t value_count_device = RADIO_DEVICE_COUNT;

    if(subghz_txrx_radio_device_get(subghz->txrx) == SubGhzRadioDeviceTypeInternal &&
       !subghz_scene_radio_settings_next_index_connect_ext_device(subghz, 1))
        value_count_device = 1;

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

    view_dispatcher_switch_to_view(subghz->view_dispatcher, SubGhzViewIdVariableItemList);
}

bool subghz_scene_radio_settings_on_event(void* context, SceneManagerEvent event) {
    SubGhz* subghz = context;
    bool consumed = false;
    UNUSED(subghz);
    UNUSED(event);

    return consumed;
}

void subghz_scene_radio_settings_on_exit(void* context) {
    SubGhz* subghz = context;
    variable_item_list_set_selected_item(subghz->variable_item_list, 0);
    variable_item_list_reset(subghz->variable_item_list);
}
