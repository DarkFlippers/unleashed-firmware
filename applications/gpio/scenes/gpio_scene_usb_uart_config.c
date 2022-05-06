#include "../usb_uart_bridge.h"
#include "../gpio_app_i.h"
#include "furi_hal.h"

typedef enum {
    UsbUartLineIndexVcp,
    UsbUartLineIndexBaudrate,
    UsbUartLineIndexUart,
    UsbUartLineIndexFlow,
} LineIndex;

static UsbUartConfig* cfg_set;

static const char* vcp_ch[] = {"0 (CLI)", "1"};
static const char* uart_ch[] = {"13,14", "15,16"};
static const char* flow_pins[] = {"None", "2,3", "6,7"};
static const char* baudrate_mode[] = {"Host"};
static const uint32_t baudrate_list[] = {
    2400,
    9600,
    19200,
    38400,
    57600,
    115200,
    230400,
    460800,
    921600,
};

bool gpio_scene_usb_uart_cfg_on_event(void* context, SceneManagerEvent event) {
    UNUSED(context);
    UNUSED(event);
    return false;
}

static void line_vcp_cb(VariableItem* item) {
    GpioApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, vcp_ch[index]);

    cfg_set->vcp_ch = index;
    usb_uart_set_config(app->usb_uart_bridge, cfg_set);
}

static void line_port_cb(VariableItem* item) {
    GpioApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, uart_ch[index]);

    if(index == 0)
        cfg_set->uart_ch = FuriHalUartIdUSART1;
    else if(index == 1)
        cfg_set->uart_ch = FuriHalUartIdLPUART1;
    usb_uart_set_config(app->usb_uart_bridge, cfg_set);
}

static void line_flow_cb(VariableItem* item) {
    GpioApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, flow_pins[index]);

    cfg_set->flow_pins = index;
    usb_uart_set_config(app->usb_uart_bridge, cfg_set);
}

static void line_baudrate_cb(VariableItem* item) {
    GpioApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    char br_text[8];

    if(index > 0) {
        snprintf(br_text, 7, "%lu", baudrate_list[index - 1]);
        variable_item_set_current_value_text(item, br_text);
        cfg_set->baudrate = baudrate_list[index - 1];
    } else {
        variable_item_set_current_value_text(item, baudrate_mode[index]);
        cfg_set->baudrate = 0;
    }
    cfg_set->baudrate_mode = index;
    usb_uart_set_config(app->usb_uart_bridge, cfg_set);
}

void gpio_scene_usb_uart_cfg_on_enter(void* context) {
    GpioApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    cfg_set = malloc(sizeof(UsbUartConfig));
    usb_uart_get_config(app->usb_uart_bridge, cfg_set);

    VariableItem* item;
    char br_text[8];

    item = variable_item_list_add(var_item_list, "USB Channel", 2, line_vcp_cb, app);
    variable_item_set_current_value_index(item, cfg_set->vcp_ch);
    variable_item_set_current_value_text(item, vcp_ch[cfg_set->vcp_ch]);

    item = variable_item_list_add(
        var_item_list,
        "Baudrate",
        sizeof(baudrate_list) / sizeof(baudrate_list[0]) + 1,
        line_baudrate_cb,
        app);
    variable_item_set_current_value_index(item, cfg_set->baudrate_mode);
    if(cfg_set->baudrate_mode > 0) {
        snprintf(br_text, 7, "%lu", baudrate_list[cfg_set->baudrate_mode - 1]);
        variable_item_set_current_value_text(item, br_text);
    } else {
        variable_item_set_current_value_text(item, baudrate_mode[cfg_set->baudrate_mode]);
    }

    item = variable_item_list_add(var_item_list, "UART Pins", 2, line_port_cb, app);
    variable_item_set_current_value_index(item, cfg_set->uart_ch);
    variable_item_set_current_value_text(item, uart_ch[cfg_set->uart_ch]);

    item = variable_item_list_add(var_item_list, "RTS/DTR Pins", 3, line_flow_cb, app);
    variable_item_set_current_value_index(item, cfg_set->flow_pins);
    variable_item_set_current_value_text(item, flow_pins[cfg_set->flow_pins]);

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, GpioAppViewUsbUartCfg));

    view_dispatcher_switch_to_view(app->view_dispatcher, GpioAppViewUsbUartCfg);
}

void gpio_scene_usb_uart_cfg_on_exit(void* context) {
    GpioApp* app = context;
    scene_manager_set_scene_state(
        app->scene_manager,
        GpioAppViewUsbUartCfg,
        variable_item_list_get_selected_item_index(app->var_item_list));
    variable_item_list_reset(app->var_item_list);
    free(cfg_set);
}
