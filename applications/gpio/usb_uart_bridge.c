#include "usb_uart_bridge.h"
#include "furi-hal.h"
#include <stream_buffer.h>
#include <furi-hal-usb-cdc_i.h>
#include "usb_cdc.h"

#define USB_CDC_PKT_LEN CDC_DATA_SZ
#define USB_UART_RX_BUF_SIZE (USB_CDC_PKT_LEN * 5)

typedef enum {
    WorkerEvtReserved = (1 << 0), // Reserved for StreamBuffer internal event
    WorkerEvtStop = (1 << 1),
    WorkerEvtRxDone = (1 << 2),

    WorkerEvtTxStop = (1 << 3),
    WorkerEvtCdcRx = (1 << 4),
} WorkerEvtFlags;

#define WORKER_ALL_RX_EVENTS (WorkerEvtStop | WorkerEvtRxDone)
#define WORKER_ALL_TX_EVENTS (WorkerEvtTxStop | WorkerEvtCdcRx)

typedef struct {
    UsbUartConfig cfg;

    FuriThread* thread;
    FuriThread* tx_thread;

    StreamBufferHandle_t rx_stream;

    osMutexId_t usb_mutex;

    osSemaphoreId_t tx_sem;

    uint8_t rx_buf[USB_CDC_PKT_LEN];

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
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        osThreadFlagsSet(furi_thread_get_thread_id(usb_uart->thread), WorkerEvtRxDone);
    }
}

static int32_t usb_uart_worker(void* context) {
    memcpy(&usb_uart->cfg, context, sizeof(UsbUartConfig));

    usb_uart->rx_stream = xStreamBufferCreate(USB_UART_RX_BUF_SIZE, 1);

    usb_uart->tx_sem = osSemaphoreNew(1, 1, NULL);
    usb_uart->usb_mutex = osMutexNew(NULL);

    usb_uart->tx_thread = furi_thread_alloc();
    furi_thread_set_name(usb_uart->tx_thread, "UsbUartTxWorker");
    furi_thread_set_stack_size(usb_uart->tx_thread, 512);
    furi_thread_set_context(usb_uart->tx_thread, NULL);
    furi_thread_set_callback(usb_uart->tx_thread, usb_uart_tx_thread);

    UsbMode usb_mode_prev = furi_hal_usb_get_config();
    if(usb_uart->cfg.vcp_ch == 0) {
        furi_hal_usb_set_config(UsbModeVcpSingle);
        furi_hal_vcp_disable();
    } else {
        furi_hal_usb_set_config(UsbModeVcpDual);
    }
    osThreadFlagsSet(furi_thread_get_thread_id(usb_uart->tx_thread), WorkerEvtCdcRx);

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
        uint32_t events = osThreadFlagsWait(WORKER_ALL_RX_EVENTS, osFlagsWaitAny, osWaitForever);
        furi_check((events & osFlagsError) == 0);
        if(events & WorkerEvtStop) break;
        if(events & WorkerEvtRxDone) {
            size_t len =
                xStreamBufferReceive(usb_uart->rx_stream, usb_uart->rx_buf, USB_CDC_PKT_LEN, 0);
            if(len > 0) {
                if(osSemaphoreAcquire(usb_uart->tx_sem, 100) == osOK) {
                    furi_check(osMutexAcquire(usb_uart->usb_mutex, osWaitForever) == osOK);
                    furi_hal_cdc_send(usb_uart->cfg.vcp_ch, usb_uart->rx_buf, len);
                    furi_check(osMutexRelease(usb_uart->usb_mutex) == osOK);
                } else {
                    xStreamBufferReset(usb_uart->rx_stream);
                }
            }
        }
    }

    osThreadFlagsSet(furi_thread_get_thread_id(usb_uart->tx_thread), WorkerEvtTxStop);
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
    osMutexDelete(usb_uart->usb_mutex);
    osSemaphoreDelete(usb_uart->tx_sem);

    return 0;
}

static int32_t usb_uart_tx_thread(void* context) {
    uint8_t data[USB_CDC_PKT_LEN];
    while(1) {
        uint32_t events = osThreadFlagsWait(WORKER_ALL_TX_EVENTS, osFlagsWaitAny, osWaitForever);
        furi_check((events & osFlagsError) == 0);
        if(events & WorkerEvtTxStop) break;
        if(events & WorkerEvtCdcRx) {
            furi_check(osMutexAcquire(usb_uart->usb_mutex, osWaitForever) == osOK);
            int32_t size = furi_hal_cdc_receive(usb_uart->cfg.vcp_ch, data, USB_CDC_PKT_LEN);
            furi_check(osMutexRelease(usb_uart->usb_mutex) == osOK);

            if(size > 0) {
                furi_hal_uart_tx(usb_uart->cfg.uart_ch, data, size);
            }
        }
    }
    return 0;
}

/* VCP callbacks */

static void vcp_on_cdc_tx_complete() {
    osSemaphoreRelease(usb_uart->tx_sem);
}

static void vcp_on_cdc_rx() {
    osThreadFlagsSet(furi_thread_get_thread_id(usb_uart->tx_thread), WorkerEvtCdcRx);
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
        furi_thread_set_name(usb_uart->thread, "UsbUartWorker");
        furi_thread_set_stack_size(usb_uart->thread, 1024);
        furi_thread_set_context(usb_uart->thread, cfg);
        furi_thread_set_callback(usb_uart->thread, usb_uart_worker);

        furi_thread_start(usb_uart->thread);
    }
}

void usb_uart_disable() {
    if(running == true) {
        osThreadFlagsSet(furi_thread_get_thread_id(usb_uart->thread), WorkerEvtStop);
        furi_thread_join(usb_uart->thread);
        furi_thread_free(usb_uart->thread);
        free(usb_uart);
        running = false;
    }
}
