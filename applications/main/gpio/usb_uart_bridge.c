#include "usb_uart_bridge.h"
#include "usb_cdc.h"
#include <cli/cli_vcp.h>
#include <cli/cli.h>
#include <toolbox/api_lock.h>
#include <furi_hal.h>
#include <furi_hal_usb_cdc.h>

#define USB_CDC_PKT_LEN CDC_DATA_SZ
#define USB_UART_RX_BUF_SIZE (USB_CDC_PKT_LEN * 5)

#define USB_CDC_BIT_DTR (1 << 0)
#define USB_CDC_BIT_RTS (1 << 1)

static const GpioPin* flow_pins[][2] = {
    {&gpio_ext_pa7, &gpio_ext_pa6}, // 2, 3
    {&gpio_ext_pb2, &gpio_ext_pc3}, // 6, 7
    {&gpio_ext_pc0, &gpio_ext_pc1}, // 16, 15
};

typedef enum {
    WorkerEvtStop = (1 << 0),
    WorkerEvtRxDone = (1 << 1),

    WorkerEvtTxStop = (1 << 2),
    WorkerEvtCdcRx = (1 << 3),

    WorkerEvtCfgChange = (1 << 4),

    WorkerEvtLineCfgSet = (1 << 5),
    WorkerEvtCtrlLineSet = (1 << 6),

} WorkerEvtFlags;

#define WORKER_ALL_RX_EVENTS                                                      \
    (WorkerEvtStop | WorkerEvtRxDone | WorkerEvtCfgChange | WorkerEvtLineCfgSet | \
     WorkerEvtCtrlLineSet)
#define WORKER_ALL_TX_EVENTS (WorkerEvtTxStop | WorkerEvtCdcRx)

struct UsbUartBridge {
    UsbUartConfig cfg;
    UsbUartConfig cfg_new;

    FuriThread* thread;
    FuriThread* tx_thread;

    FuriStreamBuffer* rx_stream;

    FuriMutex* usb_mutex;

    FuriSemaphore* tx_sem;

    UsbUartState st;

    FuriApiLock cfg_lock;

    uint8_t rx_buf[USB_CDC_PKT_LEN];
};

static void vcp_on_cdc_tx_complete(void* context);
static void vcp_on_cdc_rx(void* context);
static void vcp_state_callback(void* context, uint8_t state);
static void vcp_on_cdc_control_line(void* context, uint8_t state);
static void vcp_on_line_config(void* context, struct usb_cdc_line_coding* config);

static const CdcCallbacks cdc_cb = {
    vcp_on_cdc_tx_complete,
    vcp_on_cdc_rx,
    vcp_state_callback,
    vcp_on_cdc_control_line,
    vcp_on_line_config,
};

/* USB UART worker */

static int32_t usb_uart_tx_thread(void* context);

static void usb_uart_on_irq_cb(UartIrqEvent ev, uint8_t data, void* context) {
    UsbUartBridge* usb_uart = (UsbUartBridge*)context;

    if(ev == UartIrqEventRXNE) {
        furi_stream_buffer_send(usb_uart->rx_stream, &data, 1, 0);
        furi_thread_flags_set(furi_thread_get_id(usb_uart->thread), WorkerEvtRxDone);
    }
}

static void usb_uart_vcp_init(UsbUartBridge* usb_uart, uint8_t vcp_ch) {
    furi_hal_usb_unlock();
    if(vcp_ch == 0) {
        Cli* cli = furi_record_open(RECORD_CLI);
        cli_session_close(cli);
        furi_record_close(RECORD_CLI);
        furi_check(furi_hal_usb_set_config(&usb_cdc_single, NULL) == true);
    } else {
        furi_check(furi_hal_usb_set_config(&usb_cdc_dual, NULL) == true);
        Cli* cli = furi_record_open(RECORD_CLI);
        cli_session_open(cli, &cli_vcp);
        furi_record_close(RECORD_CLI);
    }
    furi_hal_cdc_set_callbacks(vcp_ch, (CdcCallbacks*)&cdc_cb, usb_uart);
}

static void usb_uart_vcp_deinit(UsbUartBridge* usb_uart, uint8_t vcp_ch) {
    UNUSED(usb_uart);
    furi_hal_cdc_set_callbacks(vcp_ch, NULL, NULL);
    if(vcp_ch != 0) {
        Cli* cli = furi_record_open(RECORD_CLI);
        cli_session_close(cli);
        furi_record_close(RECORD_CLI);
    }
}

static void usb_uart_serial_init(UsbUartBridge* usb_uart, uint8_t uart_ch) {
    if(uart_ch == FuriHalUartIdUSART1) {
        furi_hal_console_disable();
    } else if(uart_ch == FuriHalUartIdLPUART1) {
        furi_hal_uart_init(uart_ch, 115200);
    }
    furi_hal_uart_set_irq_cb(uart_ch, usb_uart_on_irq_cb, usb_uart);
}

static void usb_uart_serial_deinit(UsbUartBridge* usb_uart, uint8_t uart_ch) {
    UNUSED(usb_uart);
    furi_hal_uart_set_irq_cb(uart_ch, NULL, NULL);
    if(uart_ch == FuriHalUartIdUSART1)
        furi_hal_console_enable();
    else if(uart_ch == FuriHalUartIdLPUART1)
        furi_hal_uart_deinit(uart_ch);
}

static void usb_uart_set_baudrate(UsbUartBridge* usb_uart, uint32_t baudrate) {
    if(baudrate != 0) {
        furi_hal_uart_set_br(usb_uart->cfg.uart_ch, baudrate);
        usb_uart->st.baudrate_cur = baudrate;
    } else {
        struct usb_cdc_line_coding* line_cfg =
            furi_hal_cdc_get_port_settings(usb_uart->cfg.vcp_ch);
        if(line_cfg->dwDTERate > 0) {
            furi_hal_uart_set_br(usb_uart->cfg.uart_ch, line_cfg->dwDTERate);
            usb_uart->st.baudrate_cur = line_cfg->dwDTERate;
        }
    }
}

static void usb_uart_update_ctrl_lines(UsbUartBridge* usb_uart) {
    if(usb_uart->cfg.flow_pins != 0) {
        furi_assert((size_t)(usb_uart->cfg.flow_pins - 1) < COUNT_OF(flow_pins));
        uint8_t state = furi_hal_cdc_get_ctrl_line_state(usb_uart->cfg.vcp_ch);

        furi_hal_gpio_write(flow_pins[usb_uart->cfg.flow_pins - 1][0], !(state & USB_CDC_BIT_RTS));
        furi_hal_gpio_write(flow_pins[usb_uart->cfg.flow_pins - 1][1], !(state & USB_CDC_BIT_DTR));
    }
}

static int32_t usb_uart_worker(void* context) {
    UsbUartBridge* usb_uart = (UsbUartBridge*)context;

    memcpy(&usb_uart->cfg, &usb_uart->cfg_new, sizeof(UsbUartConfig));

    usb_uart->rx_stream = furi_stream_buffer_alloc(USB_UART_RX_BUF_SIZE, 1);

    usb_uart->tx_sem = furi_semaphore_alloc(1, 1);
    usb_uart->usb_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    usb_uart->tx_thread =
        furi_thread_alloc_ex("UsbUartTxWorker", 512, usb_uart_tx_thread, usb_uart);

    usb_uart_vcp_init(usb_uart, usb_uart->cfg.vcp_ch);
    usb_uart_serial_init(usb_uart, usb_uart->cfg.uart_ch);
    usb_uart_set_baudrate(usb_uart, usb_uart->cfg.baudrate);
    if(usb_uart->cfg.flow_pins != 0) {
        furi_assert((size_t)(usb_uart->cfg.flow_pins - 1) < COUNT_OF(flow_pins));
        furi_hal_gpio_init_simple(
            flow_pins[usb_uart->cfg.flow_pins - 1][0], GpioModeOutputPushPull);
        furi_hal_gpio_init_simple(
            flow_pins[usb_uart->cfg.flow_pins - 1][1], GpioModeOutputPushPull);
        usb_uart_update_ctrl_lines(usb_uart);
    }

    furi_thread_flags_set(furi_thread_get_id(usb_uart->tx_thread), WorkerEvtCdcRx);

    furi_thread_start(usb_uart->tx_thread);

    while(1) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_RX_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        furi_check(!(events & FuriFlagError));
        if(events & WorkerEvtStop) break;
        if(events & WorkerEvtRxDone) {
            size_t len = furi_stream_buffer_receive(
                usb_uart->rx_stream, usb_uart->rx_buf, USB_CDC_PKT_LEN, 0);
            if(len > 0) {
                if(furi_semaphore_acquire(usb_uart->tx_sem, 100) == FuriStatusOk) {
                    usb_uart->st.rx_cnt += len;
                    furi_check(
                        furi_mutex_acquire(usb_uart->usb_mutex, FuriWaitForever) == FuriStatusOk);
                    furi_hal_cdc_send(usb_uart->cfg.vcp_ch, usb_uart->rx_buf, len);
                    furi_check(furi_mutex_release(usb_uart->usb_mutex) == FuriStatusOk);
                } else {
                    furi_stream_buffer_reset(usb_uart->rx_stream);
                }
            }
        }
        if(events & WorkerEvtCfgChange) {
            if(usb_uart->cfg.vcp_ch != usb_uart->cfg_new.vcp_ch) {
                furi_thread_flags_set(furi_thread_get_id(usb_uart->tx_thread), WorkerEvtTxStop);
                furi_thread_join(usb_uart->tx_thread);

                usb_uart_vcp_deinit(usb_uart, usb_uart->cfg.vcp_ch);
                usb_uart_vcp_init(usb_uart, usb_uart->cfg_new.vcp_ch);

                usb_uart->cfg.vcp_ch = usb_uart->cfg_new.vcp_ch;
                furi_thread_start(usb_uart->tx_thread);
                events |= WorkerEvtCtrlLineSet;
                events |= WorkerEvtLineCfgSet;
            }
            if(usb_uart->cfg.uart_ch != usb_uart->cfg_new.uart_ch) {
                furi_thread_flags_set(furi_thread_get_id(usb_uart->tx_thread), WorkerEvtTxStop);
                furi_thread_join(usb_uart->tx_thread);

                usb_uart_serial_deinit(usb_uart, usb_uart->cfg.uart_ch);
                usb_uart_serial_init(usb_uart, usb_uart->cfg_new.uart_ch);

                usb_uart->cfg.uart_ch = usb_uart->cfg_new.uart_ch;
                usb_uart_set_baudrate(usb_uart, usb_uart->cfg.baudrate);

                furi_thread_start(usb_uart->tx_thread);
            }
            if(usb_uart->cfg.baudrate != usb_uart->cfg_new.baudrate) {
                usb_uart_set_baudrate(usb_uart, usb_uart->cfg_new.baudrate);
                usb_uart->cfg.baudrate = usb_uart->cfg_new.baudrate;
            }
            if(usb_uart->cfg.flow_pins != usb_uart->cfg_new.flow_pins) {
                if(usb_uart->cfg.flow_pins != 0) {
                    furi_hal_gpio_init_simple(
                        flow_pins[usb_uart->cfg.flow_pins - 1][0], GpioModeAnalog);
                    furi_hal_gpio_init_simple(
                        flow_pins[usb_uart->cfg.flow_pins - 1][1], GpioModeAnalog);
                }
                if(usb_uart->cfg_new.flow_pins != 0) {
                    furi_assert((size_t)(usb_uart->cfg_new.flow_pins - 1) < COUNT_OF(flow_pins));
                    furi_hal_gpio_init_simple(
                        flow_pins[usb_uart->cfg_new.flow_pins - 1][0], GpioModeOutputPushPull);
                    furi_hal_gpio_init_simple(
                        flow_pins[usb_uart->cfg_new.flow_pins - 1][1], GpioModeOutputPushPull);
                }
                usb_uart->cfg.flow_pins = usb_uart->cfg_new.flow_pins;
                events |= WorkerEvtCtrlLineSet;
            }
            api_lock_unlock(usb_uart->cfg_lock);
        }
        if(events & WorkerEvtLineCfgSet) {
            if(usb_uart->cfg.baudrate == 0)
                usb_uart_set_baudrate(usb_uart, usb_uart->cfg.baudrate);
        }
        if(events & WorkerEvtCtrlLineSet) {
            usb_uart_update_ctrl_lines(usb_uart);
        }
    }
    usb_uart_vcp_deinit(usb_uart, usb_uart->cfg.vcp_ch);
    usb_uart_serial_deinit(usb_uart, usb_uart->cfg.uart_ch);

    if(usb_uart->cfg.flow_pins != 0) {
        furi_hal_gpio_init_simple(flow_pins[usb_uart->cfg.flow_pins - 1][0], GpioModeAnalog);
        furi_hal_gpio_init_simple(flow_pins[usb_uart->cfg.flow_pins - 1][1], GpioModeAnalog);
    }

    furi_thread_flags_set(furi_thread_get_id(usb_uart->tx_thread), WorkerEvtTxStop);
    furi_thread_join(usb_uart->tx_thread);
    furi_thread_free(usb_uart->tx_thread);

    furi_stream_buffer_free(usb_uart->rx_stream);
    furi_mutex_free(usb_uart->usb_mutex);
    furi_semaphore_free(usb_uart->tx_sem);

    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_cdc_single, NULL) == true);
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_session_open(cli, &cli_vcp);
    furi_record_close(RECORD_CLI);

    return 0;
}

static int32_t usb_uart_tx_thread(void* context) {
    UsbUartBridge* usb_uart = (UsbUartBridge*)context;

    uint8_t data[USB_CDC_PKT_LEN];
    while(1) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_TX_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        furi_check(!(events & FuriFlagError));
        if(events & WorkerEvtTxStop) break;
        if(events & WorkerEvtCdcRx) {
            furi_check(furi_mutex_acquire(usb_uart->usb_mutex, FuriWaitForever) == FuriStatusOk);
            size_t len = furi_hal_cdc_receive(usb_uart->cfg.vcp_ch, data, USB_CDC_PKT_LEN);
            furi_check(furi_mutex_release(usb_uart->usb_mutex) == FuriStatusOk);

            if(len > 0) {
                usb_uart->st.tx_cnt += len;
                furi_hal_uart_tx(usb_uart->cfg.uart_ch, data, len);
            }
        }
    }
    return 0;
}

/* VCP callbacks */

static void vcp_on_cdc_tx_complete(void* context) {
    UsbUartBridge* usb_uart = (UsbUartBridge*)context;
    furi_semaphore_release(usb_uart->tx_sem);
}

static void vcp_on_cdc_rx(void* context) {
    UsbUartBridge* usb_uart = (UsbUartBridge*)context;
    furi_thread_flags_set(furi_thread_get_id(usb_uart->tx_thread), WorkerEvtCdcRx);
}

static void vcp_state_callback(void* context, uint8_t state) {
    UNUSED(context);
    UNUSED(state);
}

static void vcp_on_cdc_control_line(void* context, uint8_t state) {
    UNUSED(state);
    UsbUartBridge* usb_uart = (UsbUartBridge*)context;
    furi_thread_flags_set(furi_thread_get_id(usb_uart->thread), WorkerEvtCtrlLineSet);
}

static void vcp_on_line_config(void* context, struct usb_cdc_line_coding* config) {
    UNUSED(config);
    UsbUartBridge* usb_uart = (UsbUartBridge*)context;
    furi_thread_flags_set(furi_thread_get_id(usb_uart->thread), WorkerEvtLineCfgSet);
}

UsbUartBridge* usb_uart_enable(UsbUartConfig* cfg) {
    UsbUartBridge* usb_uart = malloc(sizeof(UsbUartBridge));

    memcpy(&(usb_uart->cfg_new), cfg, sizeof(UsbUartConfig));

    usb_uart->thread = furi_thread_alloc_ex("UsbUartWorker", 1024, usb_uart_worker, usb_uart);

    furi_thread_start(usb_uart->thread);
    return usb_uart;
}

void usb_uart_disable(UsbUartBridge* usb_uart) {
    furi_assert(usb_uart);
    furi_thread_flags_set(furi_thread_get_id(usb_uart->thread), WorkerEvtStop);
    furi_thread_join(usb_uart->thread);
    furi_thread_free(usb_uart->thread);
    free(usb_uart);
}

void usb_uart_set_config(UsbUartBridge* usb_uart, UsbUartConfig* cfg) {
    furi_assert(usb_uart);
    furi_assert(cfg);
    usb_uart->cfg_lock = api_lock_alloc_locked();
    memcpy(&(usb_uart->cfg_new), cfg, sizeof(UsbUartConfig));
    furi_thread_flags_set(furi_thread_get_id(usb_uart->thread), WorkerEvtCfgChange);
    api_lock_wait_unlock_and_free(usb_uart->cfg_lock);
}

void usb_uart_get_config(UsbUartBridge* usb_uart, UsbUartConfig* cfg) {
    furi_assert(usb_uart);
    furi_assert(cfg);
    memcpy(cfg, &(usb_uart->cfg_new), sizeof(UsbUartConfig));
}

void usb_uart_get_state(UsbUartBridge* usb_uart, UsbUartState* st) {
    furi_assert(usb_uart);
    furi_assert(st);
    memcpy(st, &(usb_uart->st), sizeof(UsbUartState));
}
