#include <furi-hal-usb-cdc_i.h>
#include <furi-hal-console.h>
#include <furi.h>
#include <stream_buffer.h>

#define TAG "FuriHalVcp"

#define USB_CDC_PKT_LEN CDC_DATA_SZ
#define VCP_RX_BUF_SIZE (USB_CDC_PKT_LEN * 3)
#define VCP_TX_BUF_SIZE (USB_CDC_PKT_LEN * 3)

#define VCP_IF_NUM 0

typedef enum {
    VcpEvtReserved      = (1 << 0), // Reserved for StreamBuffer internal event
    VcpEvtEnable        = (1 << 1),
    VcpEvtDisable       = (1 << 2),
    VcpEvtConnect       = (1 << 3),
    VcpEvtDisconnect    = (1 << 4),
    VcpEvtStreamRx      = (1 << 5),
    VcpEvtRx            = (1 << 6),
    VcpEvtStreamTx      = (1 << 7),
    VcpEvtTx            = (1 << 8),
} WorkerEvtFlags;

#define VCP_THREAD_FLAG_ALL (VcpEvtEnable | VcpEvtDisable | VcpEvtConnect | VcpEvtDisconnect | VcpEvtRx | VcpEvtTx | VcpEvtStreamRx | VcpEvtStreamTx)

typedef struct {
    FuriThread* thread;

    StreamBufferHandle_t tx_stream;
    StreamBufferHandle_t rx_stream;

    volatile bool connected;

    uint8_t data_buffer[USB_CDC_PKT_LEN];
} FuriHalVcp;

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
    furi_thread_set_stack_size(vcp->thread, 1024);
    furi_thread_set_callback(vcp->thread, vcp_worker);
    furi_thread_start(vcp->thread);

    FURI_LOG_I(TAG, "Init OK");
}

static int32_t vcp_worker(void* context) {
    bool enabled = true;
    bool tx_idle = false;
    size_t missed_rx = 0;

    furi_hal_cdc_set_callbacks(VCP_IF_NUM, &cdc_cb, NULL);

    while (1) {
        uint32_t flags = osThreadFlagsWait(VCP_THREAD_FLAG_ALL, osFlagsWaitAny, osWaitForever);
        furi_assert((flags & osFlagsError) == 0);

        // VCP enabled
        if((flags & VcpEvtEnable) && !enabled){
#ifdef FURI_HAL_USB_VCP_DEBUG
            FURI_LOG_D(TAG, "Enable");
#endif            
            flags |= VcpEvtTx;
            furi_hal_cdc_set_callbacks(VCP_IF_NUM, &cdc_cb, NULL);
            enabled = true;
            furi_hal_cdc_receive(VCP_IF_NUM, vcp->data_buffer, USB_CDC_PKT_LEN); // flush Rx buffer
            if (furi_hal_cdc_get_ctrl_line_state(VCP_IF_NUM) & (1 << 0)) {
                vcp->connected = true;
                xStreamBufferSend(vcp->rx_stream, &ascii_soh, 1, osWaitForever);
            }
        }

        // VCP disabled
        if((flags & VcpEvtDisable) && enabled) {
#ifdef FURI_HAL_USB_VCP_DEBUG            
            FURI_LOG_D(TAG, "Disable");
#endif            
            enabled = false;
            vcp->connected = false;
            xStreamBufferReceive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);
            xStreamBufferSend(vcp->rx_stream, &ascii_eot, 1, osWaitForever);
        }

        // VCP session opened
        if((flags & VcpEvtConnect) && enabled) {
#ifdef FURI_HAL_USB_VCP_DEBUG            
            FURI_LOG_D(TAG, "Connect");
#endif            
            if (vcp->connected == false) {
                vcp->connected = true;
                xStreamBufferSend(vcp->rx_stream, &ascii_soh, 1, osWaitForever);
            }
        }

        // VCP session closed
        if((flags & VcpEvtDisconnect) && enabled) {
#ifdef FURI_HAL_USB_VCP_DEBUG            
            FURI_LOG_D(TAG, "Disconnect");
#endif            
            if (vcp->connected == true) {
                vcp->connected = false;
                xStreamBufferReceive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);
                xStreamBufferSend(vcp->rx_stream, &ascii_eot, 1, osWaitForever);
            }
        }

        // Rx buffer was read, maybe there is enough space for new data?
        if((flags & VcpEvtStreamRx) && enabled && missed_rx > 0) {
#ifdef FURI_HAL_USB_VCP_DEBUG
            FURI_LOG_D(TAG, "StreamRx");
#endif
            if (xStreamBufferSpacesAvailable(vcp->rx_stream) >= USB_CDC_PKT_LEN) {
                flags |= VcpEvtRx;
                missed_rx--;
            }
        }

        // New data received
        if((flags & VcpEvtRx)) {
            if (xStreamBufferSpacesAvailable(vcp->rx_stream) >= USB_CDC_PKT_LEN) {
                int32_t len = furi_hal_cdc_receive(VCP_IF_NUM, vcp->data_buffer, USB_CDC_PKT_LEN);
#ifdef FURI_HAL_USB_VCP_DEBUG                
                FURI_LOG_D(TAG, "Rx %d", len);
#endif                
                if (len > 0) {
                    furi_check(xStreamBufferSend(vcp->rx_stream, vcp->data_buffer, len, osWaitForever) == len);
                }
            } else {
#ifdef FURI_HAL_USB_VCP_DEBUG                
                FURI_LOG_D(TAG, "Rx missed");
#endif                
                missed_rx++;
            }
        }

        // New data in Tx buffer
        if((flags & VcpEvtStreamTx) && enabled) {
#ifdef FURI_HAL_USB_VCP_DEBUG            
            FURI_LOG_D(TAG, "StreamTx");
#endif            
            if (tx_idle) {
                flags |= VcpEvtTx;
            }
        }

        // CDC write transfer done
        if((flags & VcpEvtTx) && enabled) {
            size_t len = xStreamBufferReceive(vcp->tx_stream, vcp->data_buffer, USB_CDC_PKT_LEN, 0);
#ifdef FURI_HAL_USB_VCP_DEBUG            
            FURI_LOG_D(TAG, "Tx %d", len);
#endif            
            if (len > 0) { // Some data left in Tx buffer. Sending it now
                tx_idle = false;
                furi_hal_cdc_send(VCP_IF_NUM, vcp->data_buffer, len);
            } else { // There is nothing to send. Set flag to start next transfer instantly
                tx_idle = true;
            }
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

#ifdef FURI_HAL_USB_VCP_DEBUG
    FURI_LOG_D(TAG, "rx %u start", size);
#endif    

    size_t rx_cnt = 0;

    while (size > 0) {
        size_t batch_size = size;
        if (batch_size > VCP_RX_BUF_SIZE)
            batch_size = VCP_RX_BUF_SIZE;

        size_t len = xStreamBufferReceive(vcp->rx_stream, buffer, batch_size, timeout);
#ifdef FURI_HAL_USB_VCP_DEBUG        
        FURI_LOG_D(TAG, "%u ", batch_size);
#endif        
        if (len == 0)
            break;
        osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtStreamRx);
        size -= len;
        buffer += len;
        rx_cnt += len;
    }

#ifdef FURI_HAL_USB_VCP_DEBUG
    FURI_LOG_D(TAG, "rx %u end", size);
#endif    
    return rx_cnt;
}

size_t furi_hal_vcp_rx(uint8_t* buffer, size_t size) {
    furi_assert(vcp);
    return furi_hal_vcp_rx_with_timeout(buffer, size, osWaitForever);
}

void furi_hal_vcp_tx(const uint8_t* buffer, size_t size) {
    furi_assert(vcp);
    furi_assert(buffer);

#ifdef FURI_HAL_USB_VCP_DEBUG
    FURI_LOG_D(TAG, "tx %u start", size);
#endif    

    while (size > 0 && vcp->connected) {
        size_t batch_size = size;
        if (batch_size > USB_CDC_PKT_LEN)
            batch_size = USB_CDC_PKT_LEN;

        xStreamBufferSend(vcp->tx_stream, buffer, batch_size, osWaitForever);
        osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtStreamTx);
#ifdef FURI_HAL_USB_VCP_DEBUG
        FURI_LOG_D(TAG, "%u ", batch_size);
#endif        

        size -= batch_size;
        buffer += batch_size;
    }

#ifdef FURI_HAL_USB_VCP_DEBUG
    FURI_LOG_D(TAG, "tx %u end", size);
#endif    
}

static void vcp_state_callback(void* context, uint8_t state) {
    if (state == 0) {
        osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtDisconnect);
    }
}

static void vcp_on_cdc_control_line(void* context, uint8_t state) {
    // bit 0: DTR state, bit 1: RTS state
    bool dtr = state & (1 << 0);

    if (dtr == true) {
        osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtConnect);
    } else {
        osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtDisconnect);
    }
}

static void vcp_on_cdc_rx(void* context) {
    uint32_t ret = osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtRx);
    furi_check((ret & osFlagsError) == 0);
}

static void vcp_on_cdc_tx_complete(void* context) {
    osThreadFlagsSet(furi_thread_get_thread_id(vcp->thread), VcpEvtTx);
}

bool furi_hal_vcp_is_connected(void) {
    furi_assert(vcp);
    return vcp->connected;
}
