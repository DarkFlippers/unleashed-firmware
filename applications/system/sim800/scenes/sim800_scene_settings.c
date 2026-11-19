#include "../sim800_app_i.h"

static void sim800_settings_port_changed(VariableItem* item) {
    Sim800App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, sim800_settings_port_pins[index]);
    app->settings.uart_port = index;
    // Reopening the port here would stall every press: it waits until we leave the scene
    scene_manager_set_scene_state(app->scene_manager, Sim800SceneSettings, 1);
}

static void sim800_settings_baud_changed(VariableItem* item) {
    Sim800App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, sim800_settings_baudrate_names[index]);
    app->settings.baudrate = sim800_settings_baudrates[index];
    scene_manager_set_scene_state(app->scene_manager, Sim800SceneSettings, 1);
}

static void sim800_settings_pwrkey_changed(VariableItem* item) {
    Sim800App* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, sim800_settings_pwrkey_names[index]);
    app->settings.pwrkey_pin = index;
}

static void sim800_settings_notify_changed(VariableItem* item) {
    Sim800App* app = variable_item_get_context(item);
    app->settings.notify = variable_item_get_current_value_index(item) == 1;
    variable_item_set_current_value_text(item, app->settings.notify ? "ON" : "OFF");
}

static void sim800_settings_echo_changed(VariableItem* item) {
    Sim800App* app = variable_item_get_context(item);
    app->settings.echo = variable_item_get_current_value_index(item) == 1;
    variable_item_set_current_value_text(item, app->settings.echo ? "ON" : "OFF");
    sim800_uart_command(app->uart, app->settings.echo ? "ATE1" : "ATE0", 1000, NULL);
}

void sim800_scene_settings_on_enter(void* context) {
    Sim800App* app = context;
    VariableItem* item;

    scene_manager_set_scene_state(app->scene_manager, Sim800SceneSettings, 0);
    variable_item_list_reset(app->var_item_list);

    item = variable_item_list_add(
        app->var_item_list, "UART", Sim800UartCount, sim800_settings_port_changed, app);
    variable_item_set_current_value_index(item, app->settings.uart_port);
    variable_item_set_current_value_text(item, sim800_settings_port_pins[app->settings.uart_port]);

    uint8_t baud_index = 0;
    for(uint8_t i = 0; i < SIM800_BAUDRATE_COUNT; i++) {
        if(sim800_settings_baudrates[i] == app->settings.baudrate) baud_index = i;
    }
    item = variable_item_list_add(
        app->var_item_list, "Baud", SIM800_BAUDRATE_COUNT, sim800_settings_baud_changed, app);
    variable_item_set_current_value_index(item, baud_index);
    variable_item_set_current_value_text(item, sim800_settings_baudrate_names[baud_index]);

    item = variable_item_list_add(
        app->var_item_list, "PWRKEY pin", SIM800_PWRKEY_COUNT, sim800_settings_pwrkey_changed, app);
    variable_item_set_current_value_index(item, app->settings.pwrkey_pin);
    variable_item_set_current_value_text(
        item, sim800_settings_pwrkey_names[app->settings.pwrkey_pin]);

    item = variable_item_list_add(
        app->var_item_list, "Notify", 2, sim800_settings_notify_changed, app);
    variable_item_set_current_value_index(item, app->settings.notify ? 1 : 0);
    variable_item_set_current_value_text(item, app->settings.notify ? "ON" : "OFF");

    item =
        variable_item_list_add(app->var_item_list, "Echo", 2, sim800_settings_echo_changed, app);
    variable_item_set_current_value_index(item, app->settings.echo ? 1 : 0);
    variable_item_set_current_value_text(item, app->settings.echo ? "ON" : "OFF");

    view_dispatcher_switch_to_view(app->view_dispatcher, Sim800ViewVarItemList);
}

bool sim800_scene_settings_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

void sim800_scene_settings_on_exit(void* context) {
    Sim800App* app = context;
    variable_item_list_reset(app->var_item_list);
    sim800_settings_save(&app->settings);

    // One reconnect on the way out, and only when the link itself changed
    if(scene_manager_get_scene_state(app->scene_manager, Sim800SceneSettings) != 0) {
        sim800_notify_result(app, sim800_app_connect(app, 3));
    }
}
