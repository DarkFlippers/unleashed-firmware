#include "usb_uart_bridge.h"
#include "furi-hal.h"
#include <stream_buffer.h>
#include <furi-hal-usb-cdc_i.h>
#include "usb_cdc.h"

#define USB_PKT_LEN CDC_DATA_SZ
#define USB_UART_RX_BUF_SIZE (USB_PKT_LEN * 3)
#define USB_UART_TX_BUF_SIZE (USB_PKT_LEN * 3)

typedef enum {
    WorkerEvtStop = (1 << 0),
    WorkerEvtRxReady = (1 << 1),

    WorkerEvtTxStop = (1 << 2),
    WorkerEvtTxReady = (1 << 3),

    WorkerEvtSof = (1 << 4),

} WorkerEvtFlags;

#define WORKER_ALL_RX_EVENTS (WorkerEvtStop | WorkerEvtRxReady)
#define WORKER_ALL_TX_EVENTS (WorkerEvtTxStop | WorkerEvtTxReady)

typedef struct {
    UsbUartConfig cfg;

    FuriThread* thread;
    FuriThread* tx_thread;

    osEventFlagsId_t events;

    StreamBufferHandle_t rx_stream;
    StreamBufferHandle_t tx_stream;

    uint8_t rx_buf[USB_PKT_LEN];
    uint8_t tx_buf[USB_PKT_LEN];

    bool buf_full;
} UsbUartParams;

static UsbUartParams* usb_uart;
static bool running = false;

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

static int32_t usb_uart_tx_thread(void* context);

static void usb_uart_on_irq_cb(UartIrqEvent ev, uint8_t data) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(ev == UartIrqEventRXNE) {
        xStreamBufferSendFromISR(usb_uart->rx_stream, &data, 1, &xHigherPriorityTaskWoken);

        size_t ret = xStreamBufferBytesAvailable(usb_uart->rx_stream);
        if(ret > USB_PKT_LEN) osEventFlagsSet(usb_uart->events, WorkerEvtRxReady);
    } else if(ev == UartIrqEventIDLE) {
        osEventFlagsSet(usb_uart->events, WorkerEvtRxReady);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static int32_t usb_uart_worker(void* context) {
    memcpy(&usb_uart->cfg, context, sizeof(UsbUartConfig));

    usb_uart->rx_stream = xStreamBufferCreate(USB_UART_RX_BUF_SIZE, 1);
    usb_uart->tx_stream = xStreamBufferCreate(USB_UART_TX_BUF_SIZE, 1);

    usb_uart->tx_thread = furi_thread_alloc();
    furi_thread_set_name(usb_uart->tx_thread, "usb_uart_tx");
    furi_thread_set_stack_size(usb_uart->tx_thread, 512);
    furi_thread_set_context(usb_uart->tx_thread, NULL);
    furi_thread_set_callback(usb_uart->tx_thread, usb_uart_tx_thread);

    UsbMode usb_mode_prev = furi_hal_usb_get_config();
    if(usb_uart->cfg.vcp_ch == 0) {
        furi_hal_usb_set_config(UsbModeVcpSingle);
        furi_hal_vcp_disable();
        osEventFlagsSet(usb_uart->events, WorkerEvtSof);
    } else {
        furi_hal_usb_set_config(UsbModeVcpDual);
    }

    if(usb_uart->cfg.uart_ch == FuriHalUartIdUSART1) {
        furi_hal_console_disable();
    } else if(usb_uart->cfg.uart_ch == FuriHalUartIdLPUART1) {
        furi_hal_uart_init(usb_uart->cfg.uart_ch, 115200);
        furi_hal_uart_set_irq_cb(usb_uart->cfg.uart_ch, usb_uart_on_irq_cb);
    }

    furi_hal_uart_set_irq_cb(usb_uart->cfg.uart_ch, usb_uart_on_irq_cb);
    if(usb_uart->cfg.baudrate != 0)
        furi_hal_uart_set_br(usb_uart->cfg.uart_ch, usb_uart->cfg.baudrate);
    else
        vcp_on_line_config(furi_hal_cdc_get_port_settings(usb_uart->cfg.vcp_ch));

    furi_hal_cdc_set_callbacks(usb_uart->cfg.vcp_ch, &cdc_cb);

    furi_thread_start(usb_uart->tx_thread);

    while(1) {
        uint32_t events = osEventFlagsWait(
            usb_uart->events, WORKER_ALL_RX_EVENTS, osFlagsWaitAny, osWaitForever);
        furi_check((events & osFlagsError) == 0);
        if(events & WorkerEvtStop) break;
        if(events & WorkerEvtRxReady) {
            size_t len = 0;
            do {
                len = xStreamBufferReceive(usb_uart->rx_stream, usb_uart->rx_buf, USB_PKT_LEN, 0);
                if(len > 0) {
                    if((osEventFlagsWait(usb_uart->events, WorkerEvtSof, osFlagsWaitAny, 100) &
                        osFlagsError) == 0)
                        furi_hal_cdc_send(usb_uart->cfg.vcp_ch, usb_uart->rx_buf, len);
                    else
                        xStreamBufferReset(usb_uart->rx_stream);
                }
            } while(len > 0);
        }
    }

    osEventFlagsSet(usb_uart->events, WorkerEvtTxStop);
    furi_thread_join(usb_uart->tx_thread);
    furi_thread_free(usb_uart->tx_thread);

    if(usb_uart->cfg.uart_ch == FuriHalUartIdUSART1)
        furi_hal_console_enable();
    else if(usb_uart->cfg.uart_ch == FuriHalUartIdLPUART1)
        furi_hal_uart_deinit(usb_uart->cfg.uart_ch);

    furi_hal_cdc_set_callbacks(usb_uart->cfg.vcp_ch, NULL);
    furi_hal_usb_set_config(usb_mode_prev);
    if(usb_uart->cfg.vcp_ch == 0) furi_hal_vcp_enable();

    vStreamBufferDelete(usb_uart->rx_stream);
    vStreamBufferDelete(usb_uart->tx_stream);

    return 0;
}

static int32_t usb_uart_tx_thread(void* context) {
    uint8_t data[USB_PKT_LEN];
    while(1) {
        uint32_t events = osEventFlagsWait(
            usb_uart->events, WORKER_ALL_TX_EVENTS, osFlagsWaitAny, osWaitForever);
        furi_check((events & osFlagsError) == 0);
        if(events & WorkerEvtTxStop) break;
        if(events & WorkerEvtTxReady) {
            size_t len = 0;
            do {
                len = xStreamBufferReceive(usb_uart->tx_stream, &data, 1, 0);
                if(len > 0) {
                    furi_hal_uart_tx(usb_uart->cfg.uart_ch, data, len);
                }
                if((usb_uart->buf_full == true) &&
                   (xStreamBufferBytesAvailable(usb_uart->tx_stream) == 0)) {
                    // Stream buffer was overflown, but now is free. Reading USB buffer to resume USB transfers
                    usb_uart->buf_full = false;
                    int32_t size = furi_hal_cdc_receive(usb_uart->cfg.vcp_ch, data, USB_PKT_LEN);
                    if(size > 0) {
                        furi_hal_uart_tx(usb_uart->cfg.uart_ch, data, size);
                    }
                }
            } while(len > 0);
        }
    }
    return 0;
}

/* VCP callbacks */

static void vcp_on_cdc_tx_complete() {
    osEventFlagsSet(usb_uart->events, WorkerEvtSof);
}

static void vcp_on_cdc_rx() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    uint16_t max_len = xStreamBufferSpacesAvailable(usb_uart->tx_stream);
    if(max_len >= USB_PKT_LEN) {
        //if(max_len > USB_PKT_LEN) max_len = USB_PKT_LEN;
        int32_t size = furi_hal_cdc_receive(usb_uart->cfg.vcp_ch, usb_uart->tx_buf, USB_PKT_LEN);
        if(size > 0) {
            size_t ret = xStreamBufferSendFromISR(
                usb_uart->tx_stream, usb_uart->tx_buf, size, &xHigherPriorityTaskWoken);
            furi_check(ret == size);
        }
    } else {
        usb_uart->buf_full = true;
    }
    osEventFlagsSet(usb_uart->events, WorkerEvtTxReady);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void vcp_state_callback(uint8_t state) {
}

static void vcp_on_cdc_control_line(uint8_t state) {
}

static void vcp_on_line_config(struct usb_cdc_line_coding* config) {
    if((usb_uart->cfg.baudrate == 0) && (config->dwDTERate != 0))
        furi_hal_uart_set_br(usb_uart->cfg.uart_ch, config->dwDTERate);
}

void usb_uart_enable(UsbUartConfig* cfg) {
    if(running == false) {
        running = true;
        usb_uart = furi_alloc(sizeof(UsbUartParams));

        usb_uart->thread = furi_thread_alloc();
        furi_thread_set_name(usb_uart->thread, "usb_uart");
        furi_thread_set_stack_size(usb_uart->thread, 1024);
        furi_thread_set_context(usb_uart->thread, cfg);
        furi_thread_set_callback(usb_uart->thread, usb_uart_worker);

        usb_uart->events = osEventFlagsNew(NULL);

        furi_thread_start(usb_uart->thread);
    }
}

void usb_uart_disable() {
    if(running == true) {
        osEventFlagsSet(usb_uart->events, WorkerEvtStop);
        furi_thread_join(usb_uart->thread);
        furi_thread_free(usb_uart->thread);
        osEventFlagsDelete(usb_uart->events);
        free(usb_uart);
        running = false;
    }
}
