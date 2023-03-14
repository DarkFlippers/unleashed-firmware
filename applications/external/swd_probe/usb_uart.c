
#include <stdlib.h>

#include "usb_uart.h"
#include "furi_hal.h"
#include <furi_hal_usb_cdc.h>
#include "usb_cdc.h"
#include "cli/cli_vcp.h"
#include <toolbox/api_lock.h>
#include "cli/cli.h"

#define USB_CDC_PKT_LEN CDC_DATA_SZ
#define USB_UART_RX_BUF_SIZE (USB_CDC_PKT_LEN * 5)

#define USB_CDC_BIT_DTR (1 << 0)
#define USB_CDC_BIT_RTS (1 << 1)

typedef enum {
    WorkerEvtStop = (1 << 0),
    WorkerEvtCdcRx = (1 << 1),
    WorkerEvtCfgChange = (1 << 2)

} WorkerEvtFlags;

#define WORKER_ALL_EVENTS (WorkerEvtStop | WorkerEvtCfgChange | WorkerEvtCdcRx)

struct UsbUart {
    UsbUartConfig cfg;
    UsbUartConfig cfg_new;

    FuriThread* thread;
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
    .tx_ep_callback = &vcp_on_cdc_tx_complete,
    .rx_ep_callback = &vcp_on_cdc_rx,
    .state_callback = &vcp_state_callback,
    .ctrl_line_callback = &vcp_on_cdc_control_line,
    .config_callback = &vcp_on_line_config};

static void usb_uart_vcp_init(UsbUart* usb_uart, uint8_t vcp_ch) {
    furi_hal_usb_unlock();

    Cli* cli = furi_record_open(RECORD_CLI);
    cli_session_close(cli);

    if(vcp_ch == 0) {
        furi_check(furi_hal_usb_set_config(&usb_cdc_single, NULL) == true);
    } else {
        furi_check(furi_hal_usb_set_config(&usb_cdc_dual, NULL) == true);
        cli_session_open(cli, &cli_vcp);
    }
    furi_record_close(RECORD_CLI);
    furi_hal_cdc_set_callbacks(vcp_ch, (CdcCallbacks*)&cdc_cb, usb_uart);
}

static void usb_uart_vcp_deinit(UsbUart* usb_uart, uint8_t vcp_ch) {
    UNUSED(usb_uart);
    furi_hal_cdc_set_callbacks(vcp_ch, NULL, NULL);
    if(vcp_ch != 0) {
        Cli* cli = furi_record_open(RECORD_CLI);
        cli_session_close(cli);
        furi_record_close(RECORD_CLI);
    }
}

bool usb_uart_tx_data(UsbUart* usb_uart, uint8_t* data, size_t length) {
    uint32_t pos = 0;
    while(pos < length) {
        size_t pkt_size = length - pos;

        if(pkt_size > USB_CDC_PKT_LEN) {
            pkt_size = USB_CDC_PKT_LEN;
        }

        if(furi_semaphore_acquire(usb_uart->tx_sem, 100) != FuriStatusOk) {
            return false;
        }
        if(furi_mutex_acquire(usb_uart->usb_mutex, 100) != FuriStatusOk) {
            furi_semaphore_release(usb_uart->tx_sem);
            return false;
        }
        furi_hal_cdc_send(usb_uart->cfg.vcp_ch, &data[pos], pkt_size);
        furi_mutex_release(usb_uart->usb_mutex);
        usb_uart->st.tx_cnt += pkt_size;
        pos += pkt_size;
    }
    return true;
}

static int32_t usb_uart_worker(void* context) {
    UsbUart* usb_uart = (UsbUart*)context;

    memcpy(&usb_uart->cfg, &usb_uart->cfg_new, sizeof(UsbUartConfig));

    usb_uart->tx_sem = furi_semaphore_alloc(1, 1);
    usb_uart->usb_mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    usb_uart_vcp_init(usb_uart, usb_uart->cfg.vcp_ch);

    uint8_t data[2 * USB_CDC_PKT_LEN];
    size_t remain = 0;

    while(1) {
        uint32_t events =
            furi_thread_flags_wait(WORKER_ALL_EVENTS, FuriFlagWaitAny, FuriWaitForever);
        furi_check(!(events & FuriFlagError));

        if(events & WorkerEvtStop) {
            break;
        }

        if(events & WorkerEvtCdcRx) {
            size_t len = 0;
            if(furi_mutex_acquire(usb_uart->usb_mutex, 100) == FuriStatusOk) {
                len = furi_hal_cdc_receive(usb_uart->cfg.vcp_ch, &data[remain], USB_CDC_PKT_LEN);
                furi_mutex_release(usb_uart->usb_mutex);
            }

            if(len > 0) {
                usb_uart->st.rx_cnt += len;
                remain += len;

                size_t handled = usb_uart->cfg.rx_data(usb_uart->cfg.rx_data_ctx, data, remain);

                memcpy(data, &data[handled], remain - handled);
                remain -= handled;
            }
        }

        if(events & WorkerEvtCfgChange) {
            if(usb_uart->cfg.vcp_ch != usb_uart->cfg_new.vcp_ch) {
                usb_uart_vcp_deinit(usb_uart, usb_uart->cfg.vcp_ch);
                usb_uart_vcp_init(usb_uart, usb_uart->cfg_new.vcp_ch);

                usb_uart->cfg.vcp_ch = usb_uart->cfg_new.vcp_ch;
            }
            api_lock_unlock(usb_uart->cfg_lock);
        }
    }
    usb_uart_vcp_deinit(usb_uart, usb_uart->cfg.vcp_ch);

    furi_mutex_free(usb_uart->usb_mutex);
    furi_semaphore_free(usb_uart->tx_sem);

    furi_hal_usb_unlock();
    furi_check(furi_hal_usb_set_config(&usb_cdc_single, NULL) == true);
    Cli* cli = furi_record_open(RECORD_CLI);
    cli_session_open(cli, &cli_vcp);
    furi_record_close(RECORD_CLI);

    return 0;
}

/* VCP callbacks */
static void vcp_on_cdc_tx_complete(void* context) {
    UsbUart* usb_uart = (UsbUart*)context;
    furi_semaphore_release(usb_uart->tx_sem);
}

static void vcp_on_cdc_rx(void* context) {
    UsbUart* usb_uart = (UsbUart*)context;
    furi_thread_flags_set(furi_thread_get_id(usb_uart->thread), WorkerEvtCdcRx);
}

static void vcp_state_callback(void* context, uint8_t state) {
    UNUSED(context);
    UNUSED(state);
}

static void vcp_on_cdc_control_line(void* context, uint8_t state) {
    UNUSED(context);
    UNUSED(state);
}

static void vcp_on_line_config(void* context, struct usb_cdc_line_coding* config) {
    UNUSED(context);
    UNUSED(config);
}

UsbUart* usb_uart_enable(UsbUartConfig* cfg) {
    UsbUart* usb_uart = malloc(sizeof(UsbUart));
    memcpy(&(usb_uart->cfg_new), cfg, sizeof(UsbUartConfig));

    usb_uart->thread = furi_thread_alloc_ex("UsbUartWorker", 1024, usb_uart_worker, usb_uart);
    furi_thread_start(usb_uart->thread);
    return usb_uart;
}

void usb_uart_disable(UsbUart* usb_uart) {
    furi_assert(usb_uart);
    furi_thread_flags_set(furi_thread_get_id(usb_uart->thread), WorkerEvtStop);
    furi_thread_join(usb_uart->thread);
    furi_thread_free(usb_uart->thread);
    free(usb_uart);
}

void usb_uart_set_config(UsbUart* usb_uart, UsbUartConfig* cfg) {
    furi_assert(usb_uart);
    furi_assert(cfg);
    usb_uart->cfg_lock = api_lock_alloc_locked();
    memcpy(&(usb_uart->cfg_new), cfg, sizeof(UsbUartConfig));
    furi_thread_flags_set(furi_thread_get_id(usb_uart->thread), WorkerEvtCfgChange);
    api_lock_wait_unlock_and_free(usb_uart->cfg_lock);
}

void usb_uart_get_config(UsbUart* usb_uart, UsbUartConfig* cfg) {
    furi_assert(usb_uart);
    furi_assert(cfg);
    memcpy(cfg, &(usb_uart->cfg_new), sizeof(UsbUartConfig));
}

void usb_uart_get_state(UsbUart* usb_uart, UsbUartState* st) {
    furi_assert(usb_uart);
    furi_assert(st);
    memcpy(st, &(usb_uart->st), sizeof(UsbUartState));
}
