#include <furi_hal_usb_cdc.h>
#include <furi_hal.h>
#include <furi.h>
#include "cli_i.h"

#define TAG "CliVcp"

#define USB_CDC_PKT_LEN CDC_DATA_SZ
#define VCP_RX_BUF_SIZE (USB_CDC_PKT_LEN * 3)
#define VCP_TX_BUF_SIZE (USB_CDC_PKT_LEN * 3)

#define VCP_IF_NUM 0

#ifdef CLI_VCP_DEBUG
#define VCP_DEBUG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define VCP_DEBUG(...)
#endif

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

    FuriStreamBuffer* tx_stream;
    FuriStreamBuffer* rx_stream;

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
        vcp->tx_stream = furi_stream_buffer_alloc(VCP_TX_BUF_SIZE, 1);
        vcp->rx_stream = furi_stream_buffer_alloc(VCP_RX_BUF_SIZE, 1);
    }
    furi_assert(vcp->thread == NULL);

    vcp->connected = false;

    vcp->thread = furi_thread_alloc_ex("CliVcpWorker", 1024, vcp_worker, NULL);
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
        furi_assert(!(flags & FuriFlagError));

        // VCP session opened
        if(flags & VcpEvtConnect) {
            VCP_DEBUG("Connect");

            if(vcp->connected == false) {
                vcp->connected = true;
                furi_stream_buffer_send(vcp->rx_stream, &ascii_soh, 1, FuriWaitForever);
            }
        }

        // VCP session closed
        if(flags & VcpEvtDisconnect) {
            VCP_DEBUG("Disconnect");

            if(vcp->connected == true) {
                vcp->connected = false;
                furi_stream_buffer_receive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);
                furi_stream_buffer_send(vcp->rx_stream, &ascii_eot, 1, FuriWaitForever);
            }
        }

        // Rx buffer was read, maybe there is enough space for new data?
        if((flags & VcpEvtStreamRx) && (missed_rx > 0)) {
            VCP_DEBUG("StreamRx");

            if(furi_stream_buffer_spaces_available(vcp->rx_stream) >= USB_CDC_PKT_LEN) {
                flags |= VcpEvtRx;
                missed_rx--;
            }
        }

        // New data received
        if(flags & VcpEvtRx) {
            if(furi_stream_buffer_spaces_available(vcp->rx_stream) >= USB_CDC_PKT_LEN) {
                int32_t len = furi_hal_cdc_receive(VCP_IF_NUM, vcp->data_buffer, USB_CDC_PKT_LEN);
                VCP_DEBUG("Rx %ld", len);

                if(len > 0) {
                    furi_check(
                        furi_stream_buffer_send(
                            vcp->rx_stream, vcp->data_buffer, len, FuriWaitForever) ==
                        (size_t)len);
                }
            } else {
                VCP_DEBUG("Rx missed");
                missed_rx++;
            }
        }

        // New data in Tx buffer
        if(flags & VcpEvtStreamTx) {
            VCP_DEBUG("StreamTx");

            if(tx_idle) {
                flags |= VcpEvtTx;
            }
        }

        // CDC write transfer done
        if(flags & VcpEvtTx) {
            size_t len =
                furi_stream_buffer_receive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);

            VCP_DEBUG("Tx %d", len);

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
            furi_stream_buffer_receive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);
            furi_stream_buffer_send(vcp->rx_stream, &ascii_eot, 1, FuriWaitForever);
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

    VCP_DEBUG("rx %u start", size);

    size_t rx_cnt = 0;

    while(size > 0) {
        size_t batch_size = size;
        if(batch_size > VCP_RX_BUF_SIZE) batch_size = VCP_RX_BUF_SIZE;

        size_t len = furi_stream_buffer_receive(vcp->rx_stream, buffer, batch_size, timeout);
        VCP_DEBUG("rx %u ", batch_size);

        if(len == 0) break;
        if(vcp->running == false) {
            // EOT command is received after VCP session close
            rx_cnt += len;
            break;
        }
        furi_thread_flags_set(furi_thread_get_id(vcp->thread), VcpEvtStreamRx);
        size -= len;
        buffer += len;
        rx_cnt += len;
    }

    VCP_DEBUG("rx %u end", size);
    return rx_cnt;
}

static void cli_vcp_tx(const uint8_t* buffer, size_t size) {
    furi_assert(vcp);
    furi_assert(buffer);

    if(vcp->running == false) {
        return;
    }

    VCP_DEBUG("tx %u start", size);

    while(size > 0 && vcp->connected) {
        size_t batch_size = size;
        if(batch_size > USB_CDC_PKT_LEN) batch_size = USB_CDC_PKT_LEN;

        furi_stream_buffer_send(vcp->tx_stream, buffer, batch_size, FuriWaitForever);
        furi_thread_flags_set(furi_thread_get_id(vcp->thread), VcpEvtStreamTx);
        VCP_DEBUG("tx %u", batch_size);

        size -= batch_size;
        buffer += batch_size;
    }

    VCP_DEBUG("tx %u end", size);
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
    furi_check(!(ret & FuriFlagError));
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
