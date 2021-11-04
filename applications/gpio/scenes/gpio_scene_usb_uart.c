#include "../usb_uart_bridge.h"
#include "../gpio_app_i.h"
#include "furi-hal.h"

typedef enum {
    UsbUartLineIndexVcp,
    UsbUartLineIndexUart,
    UsbUartLineIndexBaudrate,
    UsbUartLineIndexEnable,
    UsbUartLineIndexDisable,
} LineIndex;

static UsbUartConfig* cfg_set;

static const char* vcp_ch[] = {"0 (CLI)", "1"};
static const char* uart_ch[] = {"USART1", "LPUART1"};
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

bool gpio_scene_usb_uart_on_event(void* context, SceneManagerEvent event) {
    //GpioApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GPIO_SCENE_USB_UART_CUSTOM_EVENT_ENABLE) {
            usb_uart_enable(cfg_set);
        } else if(event.event == GPIO_SCENE_USB_UART_CUSTOM_EVENT_DISABLE) {
            usb_uart_disable();
        }
        consumed = true;
    }
    return consumed;
}

static void line_vcp_cb(VariableItem* item) {
    //GpioApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, vcp_ch[index]);

    cfg_set->vcp_ch = index;
}

static void line_port_cb(VariableItem* item) {
    //GpioApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, uart_ch[index]);

    if(index == 0)
        cfg_set->uart_ch = FuriHalUartIdUSART1;
    else if(index == 1)
        cfg_set->uart_ch = FuriHalUartIdLPUART1;
}

static void line_baudrate_cb(VariableItem* item) {
    //GpioApp* app = variable_item_get_context(item);
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
}

static void gpio_scene_usb_uart_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    GpioApp* app = context;
    if(index == UsbUartLineIndexEnable)
        view_dispatcher_send_custom_event(
            app->view_dispatcher, GPIO_SCENE_USB_UART_CUSTOM_EVENT_ENABLE);
    else if(index == UsbUartLineIndexDisable)
        view_dispatcher_send_custom_event(
            app->view_dispatcher, GPIO_SCENE_USB_UART_CUSTOM_EVENT_DISABLE);
}

void gpio_scene_usb_uart_on_enter(void* context) {
    GpioApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    cfg_set = furi_alloc(sizeof(UsbUartConfig));

    VariableItem* item;

    variable_item_list_set_enter_callback(var_item_list, gpio_scene_usb_uart_enter_callback, app);

    item = variable_item_list_add(var_item_list, "VCP Channel", 2, line_vcp_cb, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, vcp_ch[0]);

    item = variable_item_list_add(var_item_list, "UART Port", 2, line_port_cb, app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, uart_ch[0]);

    item = variable_item_list_add(
        var_item_list,
        "Baudrate",
        sizeof(baudrate_list) / sizeof(baudrate_list[0]) + 1,
        line_baudrate_cb,
        app);
    variable_item_set_current_value_index(item, 0);
    variable_item_set_current_value_text(item, baudrate_mode[0]);

    item = variable_item_list_add(var_item_list, "Enable", 0, NULL, NULL);
    item = variable_item_list_add(var_item_list, "Disable", 0, NULL, NULL);

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, GpioSceneUsbUart));

    view_dispatcher_switch_to_view(app->view_dispatcher, GpioAppViewUsbUart);
}

void gpio_scene_usb_uart_on_exit(void* context) {
    GpioApp* app = context;
    usb_uart_disable();
    scene_manager_set_scene_state(
        app->scene_manager,
        GpioSceneUsbUart,
        variable_item_list_get_selected_item_index(app->var_item_list));
    variable_item_list_clean(app->var_item_list);
    free(cfg_set);
}
