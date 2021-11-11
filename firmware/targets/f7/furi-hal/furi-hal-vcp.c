#include <furi-hal-usb-cdc_i.h>

#include <furi.h>
#include <stream_buffer.h>

#define USB_CDC_PKT_LEN CDC_DATA_SZ
#define VCP_RX_BUF_SIZE (USB_CDC_PKT_LEN * 3)
#define VCP_TX_BUF_SIZE (USB_CDC_PKT_LEN * 3)

#define VCP_IF_NUM 0

typedef enum {
    VcpEvtReserved      = (1 << 0), // Reserved for StreamBuffer internal event
    VcpEvtConnect       = (1 << 1),
    VcpEvtDisconnect    = (1 << 2),
    VcpEvtEnable        = (1 << 3),
    VcpEvtDisable       = (1 << 4),
    VcpEvtRx            = (1 << 5),
    VcpEvtTx            = (1 << 6),
    VcpEvtRxDone        = (1 << 7),
    VcpEvtTxDone        = (1 << 8),
} WorkerEvtFlags;

#define VCP_THREAD_FLAG_ALL (VcpEvtConnect | VcpEvtDisconnect | VcpEvtEnable | VcpEvtDisable | VcpEvtRx | VcpEvtTx | VcpEvtRxDone | VcpEvtTxDone)

typedef struct {
    FuriThread* thread;

    StreamBufferHandle_t tx_stream;
    StreamBufferHandle_t rx_stream;

    volatile bool connected;

    uint8_t data_buffer[USB_CDC_PKT_LEN];
} FuriHalVcp;

static int32_t vcp_worker(void* context);
static void vcp_on_cdc_tx_complete();
static void vcp_on_cdc_rx();
static void vcp_state_callback(uint8_t state);
static void vcp_on_cdc_control_line(uint8_t state);

static CdcCallbacks cdc_cb = {
    vcp_on_cdc_tx_complete,
    vcp_on_cdc_rx,
    vcp_state_callback,
    vcp_on_cdc_control_line,
    NULL,
};

static FuriHalVcp* vcp = NULL;

static const uint8_t ascii_soh = 0x01;
static const uint8_t ascii_eot = 0x04;

void furi_hal_vcp_init() {
    vcp = furi_alloc(sizeof(FuriHalVcp));
    vcp->connected = false;

    vcp->tx_stream = xStreamBufferCreate(VCP_TX_BUF_SIZE, 1);
    vcp->rx_stream = xStreamBufferCreate(VCP_RX_BUF_SIZE, 1);

    vcp->thread = furi_thread_alloc();
    furi_thread_set_name(vcp->thread, "VcpWorker");
    furi_thread_set_stack_size(vcp->thread, 512);
    furi_thread_set_callback(vcp->thread, vcp_worker);
    furi_thread_start(vcp->thread);

    FURI_LOG_I("FuriHalVcp", "Init OK");
}

static int32_t vcp_worker(void* context) {
    bool enabled = true;
    bool tx_idle = false;
    bool rx_pending = false;

    furi_hal_cdc_set_callbacks(VCP_IF_NUM, &cdc_cb);
    
    while (1) {
        uint32_t flags = osThreadFlagsWait(VCP_THREAD_FLAG_ALL, osFlagsWaitAny, osWaitForever);
        furi_assert((flags & osFlagsError) == 0);

        // New data received
        if((flags & VcpEvtRxDone) && enabled) {
            if (xStreamBufferSpacesAvailable(vcp->rx_stream) >= USB_CDC_PKT_LEN) {
                size_t len = furi_hal_cdc_receive(VCP_IF_NUM, vcp->data_buffer, USB_CDC_PKT_LEN);
                if (len > 0)
                    xStreamBufferSend(vcp->rx_stream, vcp->data_buffer, len, osWaitForever);
                else
                    rx_pending = false;
            } else
                rx_pending = true; // Buffer is full, retry later
        }

        // Rx buffer was read, maybe there is enough space for new data?
        if((flags & VcpEvtRx) && rx_pending) {
            if (xStreamBufferSpacesAvailable(vcp->rx_stream) >= USB_CDC_PKT_LEN) {
                size_t len = furi_hal_cdc_receive(VCP_IF_NUM, vcp->data_buffer, USB_CDC_PKT_LEN);
                if (len > 0)
                    xStreamBufferSend(vcp->rx_stream, vcp->data_buffer, len, osWaitForever);
                else
                    rx_pending = false;
            }
        }

        // New data in Tx buffer
        if((flags & VcpEvtTx) && enabled) {
            if (tx_idle) {
                size_t len = xStreamBufferReceive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);
                if (len > 0) {
                    tx_idle = false;
                    furi_hal_cdc_send(VCP_IF_NUM, vcp->data_buffer, len);
                }
            }
        }

        // CDC write transfer done
        if((flags & VcpEvtTxDone) && enabled) {
            size_t len = xStreamBufferReceive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);
            if (len > 0) { // Some data left in Tx buffer. Sending it now
                tx_idle = false;
                furi_hal_cdc_send(VCP_IF_NUM, vcp->data_buffer, len);
            } else { // There is nothing to send. Set flag to start next transfer instantly
                tx_idle = true;
            }
        }

        // VCP session opened
        if((flags & VcpEvtConnect) && enabled) {
            if (vcp->connected == false) {
                vcp->connected = true;
                xStreamBufferSend(vcp->rx_stream, &ascii_soh, 1, osWaitForever);
            }
        }

        // VCP session closed
        if((flags & VcpEvtDisconnect) && enabled) {
            if (vcp->connected == true) {
                vcp->connected = false;
                xStreamBufferSend(vcp->rx_stream, &ascii_eot, 1, osWaitForever);
            }
        }

        // VCP enabled
        if((flags & VcpEvtEnable) && !enabled){
            furi_hal_cdc_set_callbacks(VCP_IF_NUM, &cdc_cb);
            enabled = true;
            furi_hal_cdc_receive(VCP_IF_NUM, vcp->data_buffer, USB_CDC_PKT_LEN); // flush Rx buffer
            if (furi_hal_cdc_get_ctrl_line_state(VCP_IF_NUM) & (1 << 0)) {
                vcp->connected = true;
                xStreamBufferSend(vcp->rx_stream, &ascii_soh, 1, osWaitForever);
            }
        }

        // VCP disabled
        if((flags & VcpEvtDisable) && enabled) {
            enabled = false;
            vcp->connected = false;
            xStreamBufferSend(vcp->rx_stream, &ascii_eot, 1, osWaitForever);
        }
    }
    return 0;
}

void furi_hal_vcp_enable() {
    osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtEnable);
}

void furi_hal_vcp_disable() {
    osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtDisable);
}

size_t furi_hal_vcp_rx_with_timeout(uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(vcp);
    furi_assert(buffer);

    size_t rx_cnt = 0;

    while (size > 0) {
        size_t batch_size = size;
        if (batch_size > VCP_RX_BUF_SIZE)
            batch_size = VCP_RX_BUF_SIZE;

        size_t len = xStreamBufferReceive(vcp->rx_stream, buffer, batch_size, timeout);
        osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtRx);
        if (len == 0)
            break;
        size -= len;
        buffer += len;
        rx_cnt += len;
    }
    return rx_cnt;
}

size_t furi_hal_vcp_rx(uint8_t* buffer, size_t size) {
    furi_assert(vcp);
    return furi_hal_vcp_rx_with_timeout(buffer, size, osWaitForever);
}

void furi_hal_vcp_tx(const uint8_t* buffer, size_t size) {
    furi_assert(vcp);
    furi_assert(buffer);

    while (size > 0) {
        size_t batch_size = size;
        if (batch_size > VCP_TX_BUF_SIZE)
            batch_size = VCP_TX_BUF_SIZE;

        xStreamBufferSend(vcp->tx_stream, buffer, batch_size, osWaitForever);
        osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtTx);

        size -= batch_size;
        buffer += batch_size;
    }
}

static void vcp_state_callback(uint8_t state) {
    if (state == 0) {
        osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtDisconnect);
    }
}

static void vcp_on_cdc_control_line(uint8_t state) {
    // bit 0: DTR state, bit 1: RTS state
    bool dtr = state & (1 << 0);

    if (dtr == true) {
        osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtConnect);
    } else {
        osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtDisconnect);
    }
}

static void vcp_on_cdc_rx() {
    uint32_t ret = osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtRxDone);
    furi_assert((ret & osFlagsError) == 0);
}

static void vcp_on_cdc_tx_complete() {
    osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtTxDone);
}

bool furi_hal_vcp_is_connected(void) {
    furi_assert(vcp);
    return vcp->connected;
}

