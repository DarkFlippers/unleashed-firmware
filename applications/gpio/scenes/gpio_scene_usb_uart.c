#include "../gpio_app_i.h"
#include "furi-hal.h"
#include <stream_buffer.h>
#include <furi-hal-usb-cdc_i.h>
#include "usb_cdc.h"

#define USB_PKT_LEN CDC_DATA_SZ
#define USB_UART_RX_BUF_SIZE (USB_PKT_LEN * 3)
#define USB_UART_TX_BUF_SIZE (USB_PKT_LEN * 3)

typedef enum {
    WorkerCmdStop = (1 << 0),

} WorkerCommandFlags;

typedef enum {
    UsbUartLineIndexVcp,
    UsbUartLineIndexUart,
    UsbUartLineIndexBaudrate,
    UsbUartLineIndexEnable,
    UsbUartLineIndexDisable,
} LineIndex;

typedef enum {
    UsbUartPortUSART1 = 0,
    UsbUartPortLPUART1 = 1,
} PortIdx;

typedef struct {
    uint8_t vcp_ch;
    PortIdx uart_ch;
    uint32_t baudrate;
} UsbUartConfig;

typedef struct {
    UsbUartConfig cfg_cur;
    UsbUartConfig cfg_set;
    char br_text[8];

    bool running;
    osThreadId_t parent_thread;

    osThreadAttr_t thread_attr;
    osThreadId_t thread;

    osThreadAttr_t tx_thread_attr;
    osThreadId_t tx_thread;

    StreamBufferHandle_t rx_stream;
    osSemaphoreId_t rx_done_sem;
    osSemaphoreId_t usb_sof_sem;

    StreamBufferHandle_t tx_stream;

    uint8_t rx_buf[USB_PKT_LEN];
    uint8_t tx_buf[USB_PKT_LEN];
} UsbUartParams;

static UsbUartParams* usb_uart;

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

static void vcp_on_cdc_tx_complete();
static void vcp_on_cdc_rx();
static void vcp_state_callback(uint8_t state);
static void vcp_on_cdc_control_line(uint8_t state);
static void vcp_on_line_config(struct usb_cdc_line_coding* config);

static CdcCallbacks cdc_cb = {
    vcp_on_cdc_tx_complete,
    vcp_on_cdc_rx,
    vcp_state_callback,
    vcp_on_cdc_control_line,
    vcp_on_line_config,
};

/* USB UART worker */

static void usb_uart_tx_thread(void* context);

static void usb_uart_on_irq_cb(UartIrqEvent ev, uint8_t data) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(ev == UartIrqEventRXNE) {
        size_t ret =
            xStreamBufferSendFromISR(usb_uart->rx_stream, &data, 1, &xHigherPriorityTaskWoken);
        furi_check(ret == 1);
        ret = xStreamBufferBytesAvailable(usb_uart->rx_stream);
        if(ret > USB_PKT_LEN) osSemaphoreRelease(usb_uart->rx_done_sem);
    } else if(ev == UartIrqEventIDLE) {
        osSemaphoreRelease(usb_uart->rx_done_sem);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void usb_uart_worker(void* context) {
    memcpy(&usb_uart->cfg_cur, &usb_uart->cfg_set, sizeof(UsbUartConfig));

    usb_uart->rx_stream = xStreamBufferCreate(USB_UART_RX_BUF_SIZE, 1);
    usb_uart->rx_done_sem = osSemaphoreNew(1, 1, NULL);
    usb_uart->usb_sof_sem = osSemaphoreNew(1, 1, NULL);

    usb_uart->tx_stream = xStreamBufferCreate(USB_UART_TX_BUF_SIZE, 1);

    usb_uart->tx_thread = NULL;
    usb_uart->tx_thread_attr.name = "usb_uart_tx";
    usb_uart->tx_thread_attr.stack_size = 512;

    UsbMode usb_mode_prev = furi_hal_usb_get_config();
    if(usb_uart->cfg_cur.vcp_ch == 0) {
        furi_hal_usb_set_config(UsbModeVcpSingle);
        furi_hal_vcp_disable();
    } else {
        furi_hal_usb_set_config(UsbModeVcpDual);
    }

    if(usb_uart->cfg_cur.uart_ch == UsbUartPortUSART1) {
        furi_hal_usart_init();
        furi_hal_usart_set_irq_cb(usb_uart_on_irq_cb);
        if(usb_uart->cfg_cur.baudrate != 0)
            furi_hal_usart_set_br(usb_uart->cfg_cur.baudrate);
        else
            vcp_on_line_config(furi_hal_cdc_get_port_settings(usb_uart->cfg_cur.vcp_ch));
    } else if(usb_uart->cfg_cur.uart_ch == UsbUartPortLPUART1) {
        furi_hal_lpuart_init();
        furi_hal_lpuart_set_irq_cb(usb_uart_on_irq_cb);
        if(usb_uart->cfg_cur.baudrate != 0)
            furi_hal_lpuart_set_br(usb_uart->cfg_cur.baudrate);
        else
            vcp_on_line_config(furi_hal_cdc_get_port_settings(usb_uart->cfg_cur.vcp_ch));
    }

    furi_hal_cdc_set_callbacks(usb_uart->cfg_cur.vcp_ch, &cdc_cb);
    usb_uart->tx_thread = osThreadNew(usb_uart_tx_thread, NULL, &usb_uart->tx_thread_attr);

    while(1) {
        furi_check(osSemaphoreAcquire(usb_uart->rx_done_sem, osWaitForever) == osOK);
        if(osThreadFlagsWait(WorkerCmdStop, osFlagsWaitAny, 0) == WorkerCmdStop) break;
        size_t len = 0;
        do {
            len = xStreamBufferReceive(usb_uart->rx_stream, usb_uart->rx_buf, USB_PKT_LEN, 0);
            if(len > 0) {
                if(osSemaphoreAcquire(usb_uart->usb_sof_sem, 100) == osOK)
                    furi_hal_cdc_send(usb_uart->cfg_cur.vcp_ch, usb_uart->rx_buf, len);
                else
                    xStreamBufferReset(usb_uart->rx_stream);
            }
        } while(len > 0);
    }

    osThreadTerminate(usb_uart->tx_thread);

    if(usb_uart->cfg_cur.uart_ch == UsbUartPortUSART1)
        furi_hal_usart_deinit();
    else if(usb_uart->cfg_cur.uart_ch == UsbUartPortLPUART1)
        furi_hal_lpuart_deinit();

    furi_hal_cdc_set_callbacks(usb_uart->cfg_cur.vcp_ch, NULL);
    furi_hal_usb_set_config(usb_mode_prev);
    if(usb_uart->cfg_cur.vcp_ch == 0) furi_hal_vcp_enable();

    vStreamBufferDelete(usb_uart->rx_stream);
    osSemaphoreDelete(usb_uart->rx_done_sem);
    osSemaphoreDelete(usb_uart->usb_sof_sem);

    vStreamBufferDelete(usb_uart->tx_stream);
    osThreadFlagsSet(usb_uart->parent_thread, WorkerCmdStop);
    osThreadExit();
}

static void usb_uart_tx_thread(void* context) {
    uint8_t data = 0;
    while(1) {
        size_t len = xStreamBufferReceive(usb_uart->tx_stream, &data, 1, osWaitForever);
        if(len > 0) {
            if(usb_uart->cfg_cur.uart_ch == UsbUartPortUSART1)
                furi_hal_usart_tx(&data, len);
            else if(usb_uart->cfg_cur.uart_ch == UsbUartPortLPUART1)
                furi_hal_lpuart_tx(&data, len);
        }
    }
    osThreadExit();
}

/* VCP callbacks */

static void vcp_on_cdc_tx_complete() {
    osSemaphoreRelease(usb_uart->usb_sof_sem);
}

static void vcp_on_cdc_rx() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint16_t max_len = xStreamBufferSpacesAvailable(usb_uart->tx_stream);
    if(max_len > 0) {
        if(max_len > USB_PKT_LEN) max_len = USB_PKT_LEN;
        int32_t size = furi_hal_cdc_receive(usb_uart->cfg_cur.vcp_ch, usb_uart->tx_buf, max_len);

        if(size > 0) {
            size_t ret = xStreamBufferSendFromISR(
                usb_uart->tx_stream, usb_uart->tx_buf, size, &xHigherPriorityTaskWoken);
            furi_check(ret == size);
        }
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void vcp_state_callback(uint8_t state) {
}

static void vcp_on_cdc_control_line(uint8_t state) {
}

static void vcp_on_line_config(struct usb_cdc_line_coding* config) {
    if((usb_uart->cfg_cur.baudrate == 0) && (config->dwDTERate != 0)) {
        if(usb_uart->cfg_cur.uart_ch == UsbUartPortUSART1)
            furi_hal_usart_set_br(config->dwDTERate);
        else if(usb_uart->cfg_cur.uart_ch == UsbUartPortLPUART1)
            furi_hal_lpuart_set_br(config->dwDTERate);
    }
}

/* USB UART app */

static void usb_uart_enable() {
    if(usb_uart->running == false) {
        usb_uart->thread = NULL;
        usb_uart->thread_attr.name = "usb_uart";
        usb_uart->thread_attr.stack_size = 1024;
        usb_uart->parent_thread = osThreadGetId();
        usb_uart->running = true;
        usb_uart->thread = osThreadNew(usb_uart_worker, NULL, &usb_uart->thread_attr);
    }
}

static void usb_uart_disable() {
    if(usb_uart->running == true) {
        osThreadFlagsSet(usb_uart->thread, WorkerCmdStop);
        osSemaphoreRelease(usb_uart->rx_done_sem);
        osThreadFlagsWait(WorkerCmdStop, osFlagsWaitAny, osWaitForever);
        usb_uart->running = false;
    }
}

bool gpio_scene_usb_uart_on_event(void* context, SceneManagerEvent event) {
    //GpioApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == UsbUartLineIndexEnable) {
            usb_uart_enable();
        } else if(event.event == UsbUartLineIndexDisable) {
            usb_uart_disable();
        }
        consumed = true;
    }
    return consumed;
}

/* Scene callbacks */

static void line_vcp_cb(VariableItem* item) {
    //GpioApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, vcp_ch[index]);

    usb_uart->cfg_set.vcp_ch = index;
}

static void line_port_cb(VariableItem* item) {
    //GpioApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    variable_item_set_current_value_text(item, uart_ch[index]);

    usb_uart->cfg_set.uart_ch = index;
}

static void line_baudrate_cb(VariableItem* item) {
    //GpioApp* app = variable_item_get_context(item);
    uint8_t index = variable_item_get_current_value_index(item);

    if(index > 0) {
        snprintf(usb_uart->br_text, 7, "%lu", baudrate_list[index - 1]);
        variable_item_set_current_value_text(item, usb_uart->br_text);
        usb_uart->cfg_set.baudrate = baudrate_list[index - 1];
    } else {
        variable_item_set_current_value_text(item, baudrate_mode[index]);
        usb_uart->cfg_set.baudrate = 0;
    }
}

static void gpio_scene_usb_uart_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    GpioApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, index);
}

void gpio_scene_usb_uart_on_enter(void* context) {
    GpioApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    usb_uart = furi_alloc(sizeof(UsbUartParams));

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

    view_dispatcher_switch_to_view(app->view_dispatcher, GpioAppViewUsbUart);
}

void gpio_scene_usb_uart_on_exit(void* context) {
    GpioApp* app = context;
    usb_uart_disable();
    variable_item_list_clean(app->var_item_list);
    free(usb_uart);
}