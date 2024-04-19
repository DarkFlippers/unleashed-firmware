#include "../infrared_app_i.h"

static const char* infrared_scene_gpio_settings_pin_text[] = {
    "Flipper",
    "2 (A7)",
    "Detect",
};

static const char* infrared_scene_gpio_settings_otg_text[] = {
    "OFF",
    "ON",
};

static void infrared_scene_gpio_settings_pin_change_callback(VariableItem* item) {
    InfraredApp* infrared = variable_item_get_context(item);
    const uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, infrared_scene_gpio_settings_pin_text[index]);
    view_dispatcher_send_custom_event(
        infrared->view_dispatcher,
        infrared_custom_event_pack(InfraredCustomEventTypeGpioTxPinChanged, index));
}

static void infrared_scene_gpio_settings_otg_change_callback(VariableItem* item) {
    InfraredApp* infrared = variable_item_get_context(item);
    const uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, infrared_scene_gpio_settings_otg_text[index]);
    view_dispatcher_send_custom_event(
        infrared->view_dispatcher,
        infrared_custom_event_pack(InfraredCustomEventTypeGpioOtgChanged, index));
}

static void infrared_scene_gpio_settings_init(InfraredApp* infrared) {
    VariableItemList* var_item_list = infrared->var_item_list;
    VariableItem* item;
    uint8_t value_index;

    item = variable_item_list_add(
        var_item_list,
        "Signal Output",
        COUNT_OF(infrared_scene_gpio_settings_pin_text),
        infrared_scene_gpio_settings_pin_change_callback,
        infrared);

    value_index = infrared->app_state.tx_pin;
    variable_item_set_current_value_index(item, value_index);
    variable_item_set_current_value_text(item, infrared_scene_gpio_settings_pin_text[value_index]);

    item = variable_item_list_add(
        var_item_list,
        "5V on GPIO",
        COUNT_OF(infrared_scene_gpio_settings_otg_text),
        infrared_scene_gpio_settings_otg_change_callback,
        infrared);

    if(infrared->app_state.tx_pin < FuriHalInfraredTxPinMax) {
        value_index = infrared->app_state.is_otg_enabled;
        variable_item_set_current_value_index(item, value_index);
        variable_item_set_current_value_text(
            item, infrared_scene_gpio_settings_otg_text[value_index]);
    } else {
        variable_item_set_values_count(item, 1);
        variable_item_set_current_value_index(item, 0);
        variable_item_set_current_value_text(item, "Auto");
    }
}

void infrared_scene_gpio_settings_on_enter(void* context) {
    InfraredApp* infrared = context;
    infrared_scene_gpio_settings_init(infrared);
    view_dispatcher_switch_to_view(infrared->view_dispatcher, InfraredViewVariableList);
}

bool infrared_scene_gpio_settings_on_event(void* context, SceneManagerEvent event) {
    bool consumed = false;

    InfraredApp* infrared = context;

    if(event.type == SceneManagerEventTypeCustom) {
        const uint16_t custom_event_type = infrared_custom_event_get_type(event.event);
        const uint16_t custom_event_value = infrared_custom_event_get_value(event.event);

        if(custom_event_type == InfraredCustomEventTypeGpioTxPinChanged) {
            infrared_set_tx_pin(infrared, custom_event_value);
            variable_item_list_reset(infrared->var_item_list);
            infrared_scene_gpio_settings_init(infrared);
        } else if(custom_event_type == InfraredCustomEventTypeGpioOtgChanged) {
            infrared_enable_otg(infrared, custom_event_value);
        }

        consumed = true;
    }

    return consumed;
}

void infrared_scene_gpio_settings_on_exit(void* context) {
    InfraredApp* infrared = context;
    variable_item_list_reset(infrared->var_item_list);
    infrared_save_settings(infrared);
}
