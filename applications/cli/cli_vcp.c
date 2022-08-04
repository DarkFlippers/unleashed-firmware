#include <furi_hal_usb_cdc_i.h>
#include <furi_hal.h>
#include <furi.h>
#include <stream_buffer.h>
#include "cli_i.h"

#define TAG "CliVcp"

#define USB_CDC_PKT_LEN CDC_DATA_SZ
#define VCP_RX_BUF_SIZE (USB_CDC_PKT_LEN * 3)
#define VCP_TX_BUF_SIZE (USB_CDC_PKT_LEN * 3)

#define VCP_IF_NUM 0

typedef enum {
    VcpEvtStop = (1 << 0),
    VcpEvtConnect = (1 << 1),
    VcpEvtDisconnect = (1 << 2),
    VcpEvtStreamRx = (1 << 3),
    VcpEvtRx = (1 << 4),
    VcpEvtStreamTx = (1 << 5),
    VcpEvtTx = (1 << 6),
} WorkerEvtFlags;

#define VCP_THREAD_FLAG_ALL                                                                 \
    (VcpEvtStop | VcpEvtConnect | VcpEvtDisconnect | VcpEvtRx | VcpEvtTx | VcpEvtStreamRx | \
     VcpEvtStreamTx)

typedef struct {
    FuriThread* thread;

    StreamBufferHandle_t tx_stream;
    StreamBufferHandle_t rx_stream;

    volatile bool connected;
    volatile bool running;

    FuriHalUsbInterface* usb_if_prev;

    uint8_t data_buffer[USB_CDC_PKT_LEN];
} CliVcp;

static int32_t vcp_worker(void* context);
static void vcp_on_cdc_tx_complete(void* context);
static void vcp_on_cdc_rx(void* context);
static void vcp_state_callback(void* context, uint8_t state);
static void vcp_on_cdc_control_line(void* context, uint8_t state);

static CdcCallbacks cdc_cb = {
    vcp_on_cdc_tx_complete,
    vcp_on_cdc_rx,
    vcp_state_callback,
    vcp_on_cdc_control_line,
    NULL,
};

static CliVcp* vcp = NULL;

static const uint8_t ascii_soh = 0x01;
static const uint8_t ascii_eot = 0x04;

static void cli_vcp_init() {
    if(vcp == NULL) {
        vcp = malloc(sizeof(CliVcp));
        vcp->tx_stream = xStreamBufferCreate(VCP_TX_BUF_SIZE, 1);
        vcp->rx_stream = xStreamBufferCreate(VCP_RX_BUF_SIZE, 1);
    }
    furi_assert(vcp->thread == NULL);

    vcp->connected = false;

    vcp->thread = furi_thread_alloc();
    furi_thread_set_name(vcp->thread, "CliVcpWorker");
    furi_thread_set_stack_size(vcp->thread, 1024);
    furi_thread_set_callback(vcp->thread, vcp_worker);
    furi_thread_start(vcp->thread);

    FURI_LOG_I(TAG, "Init OK");
}

static void cli_vcp_deinit() {
    furi_thread_flags_set(furi_thread_get_id(vcp->thread), VcpEvtStop);
    furi_thread_join(vcp->thread);
    furi_thread_free(vcp->thread);
    vcp->thread = NULL;
}

static int32_t vcp_worker(void* context) {
    UNUSED(context);
    bool tx_idle = true;
    size_t missed_rx = 0;
    uint8_t last_tx_pkt_len = 0;

    // Switch USB to VCP mode (if it is not set yet)
    vcp->usb_if_prev = furi_hal_usb_get_config();
    if((vcp->usb_if_prev != &usb_cdc_single) && (vcp->usb_if_prev != &usb_cdc_dual)) {
        furi_hal_usb_set_config(&usb_cdc_single, NULL);
    }
    furi_hal_cdc_set_callbacks(VCP_IF_NUM, &cdc_cb, NULL);

    FURI_LOG_D(TAG, "Start");
    vcp->running = true;

    while(1) {
        uint32_t flags =
            furi_thread_flags_wait(VCP_THREAD_FLAG_ALL, FuriFlagWaitAny, FuriWaitForever);
        furi_assert((flags & FuriFlagError) == 0);

        // VCP session opened
        if(flags & VcpEvtConnect) {
#ifdef CLI_VCP_DEBUG
            FURI_LOG_D(TAG, "Connect");
#endif
            if(vcp->connected == false) {
                vcp->connected = true;
                xStreamBufferSend(vcp->rx_stream, &ascii_soh, 1, FuriWaitForever);
            }
        }

        // VCP session closed
        if(flags & VcpEvtDisconnect) {
#ifdef CLI_VCP_DEBUG
            FURI_LOG_D(TAG, "Disconnect");
#endif
            if(vcp->connected == true) {
                vcp->connected = false;
                xStreamBufferReceive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);
                xStreamBufferSend(vcp->rx_stream, &ascii_eot, 1, FuriWaitForever);
            }
        }

        // Rx buffer was read, maybe there is enough space for new data?
        if((flags & VcpEvtStreamRx) && (missed_rx > 0)) {
#ifdef CLI_VCP_DEBUG
            FURI_LOG_D(TAG, "StreamRx");
#endif
            if(xStreamBufferSpacesAvailable(vcp->rx_stream) >= USB_CDC_PKT_LEN) {
                flags |= VcpEvtRx;
                missed_rx--;
            }
        }

        // New data received
        if(flags & VcpEvtRx) {
            if(xStreamBufferSpacesAvailable(vcp->rx_stream) >= USB_CDC_PKT_LEN) {
                int32_t len = furi_hal_cdc_receive(VCP_IF_NUM, vcp->data_buffer, USB_CDC_PKT_LEN);
#ifdef CLI_VCP_DEBUG
                FURI_LOG_D(TAG, "Rx %d", len);
#endif
                if(len > 0) {
                    furi_check(
                        xStreamBufferSend(vcp->rx_stream, vcp->data_buffer, len, FuriWaitForever) ==
                        (size_t)len);
                }
            } else {
#ifdef CLI_VCP_DEBUG
                FURI_LOG_D(TAG, "Rx missed");
#endif
                missed_rx++;
            }
        }

        // New data in Tx buffer
        if(flags & VcpEvtStreamTx) {
#ifdef CLI_VCP_DEBUG
            FURI_LOG_D(TAG, "StreamTx");
#endif
            if(tx_idle) {
                flags |= VcpEvtTx;
            }
        }

        // CDC write transfer done
        if(flags & VcpEvtTx) {
            size_t len =
                xStreamBufferReceive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);
#ifdef CLI_VCP_DEBUG
            FURI_LOG_D(TAG, "Tx %d", len);
#endif
            if(len > 0) { // Some data left in Tx buffer. Sending it now
                tx_idle = false;
                furi_hal_cdc_send(VCP_IF_NUM, vcp->data_buffer, len);
                last_tx_pkt_len = len;
            } else { // There is nothing to send.
                if(last_tx_pkt_len == 64) {
                    // Send extra zero-length packet if last packet len is 64 to indicate transfer end
                    furi_hal_cdc_send(VCP_IF_NUM, NULL, 0);
                } else {
                    // Set flag to start next transfer instantly
                    tx_idle = true;
                }
                last_tx_pkt_len = 0;
            }
        }

        if(flags & VcpEvtStop) {
            vcp->connected = false;
            vcp->running = false;
            furi_hal_cdc_set_callbacks(VCP_IF_NUM, NULL, NULL);
            // Restore previous USB mode (if it was set during init)
            if((vcp->usb_if_prev != &usb_cdc_single) && (vcp->usb_if_prev != &usb_cdc_dual)) {
                furi_hal_usb_unlock();
                furi_hal_usb_set_config(vcp->usb_if_prev, NULL);
            }
            xStreamBufferReceive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);
            xStreamBufferSend(vcp->rx_stream, &ascii_eot, 1, FuriWaitForever);
            break;
        }
    }
    FURI_LOG_D(TAG, "End");
    return 0;
}

static size_t cli_vcp_rx(uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(vcp);
    furi_assert(buffer);

    if(vcp->running == false) {
        return 0;
    }

#ifdef CLI_VCP_DEBUG
    FURI_LOG_D(TAG, "rx %u start", size);
#endif

    size_t rx_cnt = 0;

    while(size > 0) {
        size_t batch_size = size;
        if(batch_size > VCP_RX_BUF_SIZE) batch_size = VCP_RX_BUF_SIZE;

        size_t len = xStreamBufferReceive(vcp->rx_stream, buffer, batch_size, timeout);
#ifdef CLI_VCP_DEBUG
        FURI_LOG_D(TAG, "rx %u ", batch_size);
#endif
        if(len == 0) break;
        furi_thread_flags_set(furi_thread_get_id(vcp->thread), VcpEvtStreamRx);
        size -= len;
        buffer += len;
        rx_cnt += len;
    }

#ifdef CLI_VCP_DEBUG
    FURI_LOG_D(TAG, "rx %u end", size);
#endif
    return rx_cnt;
}

static void cli_vcp_tx(const uint8_t* buffer, size_t size) {
    furi_assert(vcp);
    furi_assert(buffer);

    if(vcp->running == false) {
        return;
    }

#ifdef CLI_VCP_DEBUG
    FURI_LOG_D(TAG, "tx %u start", size);
#endif

    while(size > 0 && vcp->connected) {
        size_t batch_size = size;
        if(batch_size > USB_CDC_PKT_LEN) batch_size = USB_CDC_PKT_LEN;

        xStreamBufferSend(vcp->tx_stream, buffer, batch_size, FuriWaitForever);
        furi_thread_flags_set(furi_thread_get_id(vcp->thread), VcpEvtStreamTx);
#ifdef CLI_VCP_DEBUG
        FURI_LOG_D(TAG, "tx %u", batch_size);
#endif

        size -= batch_size;
        buffer += batch_size;
    }

#ifdef CLI_VCP_DEBUG
    FURI_LOG_D(TAG, "tx %u end", size);
#endif
}

static void cli_vcp_tx_stdout(const char* data, size_t size) {
    cli_vcp_tx((const uint8_t*)data, size);
}

static void vcp_state_callback(void* context, uint8_t state) {
    UNUSED(context);
    if(state == 0) {
        furi_thread_flags_set(furi_thread_get_id(vcp->thread), VcpEvtDisconnect);
    }
}

static void vcp_on_cdc_control_line(void* context, uint8_t state) {
    UNUSED(context);
    // bit 0: DTR state, bit 1: RTS state
    bool dtr = state & (1 << 0);

    if(dtr == true) {
        furi_thread_flags_set(furi_thread_get_id(vcp->thread), VcpEvtConnect);
    } else {
        furi_thread_flags_set(furi_thread_get_id(vcp->thread), VcpEvtDisconnect);
    }
}

static void vcp_on_cdc_rx(void* context) {
    UNUSED(context);
    uint32_t ret = furi_thread_flags_set(furi_thread_get_id(vcp->thread), VcpEvtRx);
    furi_check((ret & FuriFlagError) == 0);
}

static void vcp_on_cdc_tx_complete(void* context) {
    UNUSED(context);
    furi_thread_flags_set(furi_thread_get_id(vcp->thread), VcpEvtTx);
}

static bool cli_vcp_is_connected(void) {
    furi_assert(vcp);
    return vcp->connected;
}

CliSession cli_vcp = {
    cli_vcp_init,
    cli_vcp_deinit,
    cli_vcp_rx,
    cli_vcp_tx,
    cli_vcp_tx_stdout,
    cli_vcp_is_connected,
};
