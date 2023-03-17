#include "../dap_gui_i.h"

static const char* swd_pins[] = {[DapSwdPinsPA7PA6] = "2,3", [DapSwdPinsPA14PA13] = "10,12"};
static const char* uart_pins[] = {[DapUartTypeUSART1] = "13,14", [DapUartTypeLPUART1] = "15,16"};
static const char* uart_swap[] = {[DapUartTXRXNormal] = "No", [DapUartTXRXSwap] = "Yes"};

static void swd_pins_cb(VariableItem* item) {
    DapGuiApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, swd_pins[index]);

    DapConfig* config = dap_app_get_config(app->dap_app);
    config->swd_pins = index;
    dap_app_set_config(app->dap_app, config);
}

static void uart_pins_cb(VariableItem* item) {
    DapGuiApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, uart_pins[index]);

    DapConfig* config = dap_app_get_config(app->dap_app);
    config->uart_pins = index;
    dap_app_set_config(app->dap_app, config);
}

static void uart_swap_cb(VariableItem* item) {
    DapGuiApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, uart_swap[index]);

    DapConfig* config = dap_app_get_config(app->dap_app);
    config->uart_swap = index;
    dap_app_set_config(app->dap_app, config);
}

static void ok_cb(void* context, uint32_t index) {
    DapGuiApp* app = context;
    switch(index) {
    case 3:
        view_dispatcher_send_custom_event(app->view_dispatcher, DapAppCustomEventHelp);
        break;
    case 4:
        view_dispatcher_send_custom_event(app->view_dispatcher, DapAppCustomEventAbout);
        break;
    default:
        break;
    }
}

void dap_scene_config_on_enter(void* context) {
    DapGuiApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;
    VariableItem* item;
    DapConfig* config = dap_app_get_config(app->dap_app);

    item = variable_item_list_add(
        var_item_list, "SWC SWD Pins", COUNT_OF(swd_pins), swd_pins_cb, app);
    variable_item_set_current_value_index(item, config->swd_pins);
    variable_item_set_current_value_text(item, swd_pins[config->swd_pins]);

    item =
        variable_item_list_add(var_item_list, "UART Pins", COUNT_OF(uart_pins), uart_pins_cb, app);
    variable_item_set_current_value_index(item, config->uart_pins);
    variable_item_set_current_value_text(item, uart_pins[config->uart_pins]);

    item = variable_item_list_add(
        var_item_list, "Swap TX RX", COUNT_OF(uart_swap), uart_swap_cb, app);
    variable_item_set_current_value_index(item, config->uart_swap);
    variable_item_set_current_value_text(item, uart_swap[config->uart_swap]);

    variable_item_list_add(var_item_list, "Help and Pinout", 0, NULL, NULL);
    variable_item_list_add(var_item_list, "About", 0, NULL, NULL);

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, DapSceneConfig));

    variable_item_list_set_enter_callback(var_item_list, ok_cb, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, DapGuiAppViewVarItemList);
}

bool dap_scene_config_on_event(void* context, SceneManagerEvent event) {
    DapGuiApp* app = context;
    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == DapAppCustomEventHelp) {
            scene_manager_next_scene(app->scene_manager, DapSceneHelp);
            return true;
        } else if(event.event == DapAppCustomEventAbout) {
            scene_manager_next_scene(app->scene_manager, DapSceneAbout);
            return true;
        }
    }
    return false;
}

void dap_scene_config_on_exit(void* context) {
    DapGuiApp* app = context;
    scene_manager_set_scene_state(
        app->scene_manager,
        DapSceneConfig,
        variable_item_list_get_selected_item_index(app->var_item_list));
    variable_item_list_reset(app->var_item_list);
}