#include <furi-hal-usb-cdc_i.h>

#include <furi.h>
#include <stream_buffer.h>

#define USB_CDC_PKT_LEN CDC_DATA_SZ
#define VCP_IF_NUM 0

typedef enum {
    VcpConnect,
    VcpDisconnect,
} VcpEvent;

typedef struct {
    volatile bool connected;

    uint8_t rx_buf[USB_CDC_PKT_LEN];
    uint8_t rx_buf_start;
    uint8_t rx_buf_len;

    osMessageQueueId_t event_queue;

    osMutexId_t usb_mutex;

    osSemaphoreId_t tx_sem;
    osSemaphoreId_t rx_sem;

} FuriHalVcp;

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

    vcp->usb_mutex = osMutexNew(NULL);

    vcp->tx_sem = osSemaphoreNew(1, 1, NULL);
    vcp->rx_sem = osSemaphoreNew(1, 0, NULL);

    vcp->event_queue = osMessageQueueNew(8, sizeof(VcpEvent), NULL);

    furi_hal_cdc_set_callbacks(VCP_IF_NUM, &cdc_cb);

    FURI_LOG_I("FuriHalVcp", "Init OK");
}

void furi_hal_vcp_enable() {
    furi_hal_cdc_set_callbacks(VCP_IF_NUM, &cdc_cb);
    VcpEvent evt = VcpConnect;
    osMessageQueuePut(vcp->event_queue, &evt, 0, 0);
    vcp->connected = true;
    osSemaphoreRelease(vcp->tx_sem);
    osSemaphoreRelease(vcp->rx_sem);
}

void furi_hal_vcp_disable() {
    furi_hal_cdc_set_callbacks(VCP_IF_NUM, NULL);
    VcpEvent evt = VcpDisconnect;
    osMessageQueuePut(vcp->event_queue, &evt, 0, 0);
    vcp->connected = false;
    osSemaphoreRelease(vcp->tx_sem);
    osSemaphoreRelease(vcp->rx_sem);
}

size_t furi_hal_vcp_rx_with_timeout(uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(vcp);
    furi_assert(buffer);

    size_t rx_cnt = 0;

    VcpEvent evt = VcpDisconnect;

    if (vcp->rx_buf_len > 0) {
        size_t len = (vcp->rx_buf_len > size) ? (size) : (vcp->rx_buf_len);
        memcpy(&buffer[rx_cnt], &vcp->rx_buf[vcp->rx_buf_start], len);
        vcp->rx_buf_len -= len;
        vcp->rx_buf_start += len;
        rx_cnt += len;
    }

    while (rx_cnt < size) {
        if (osMessageQueueGet(vcp->event_queue, &evt, NULL, 0) == osOK) {
            if (evt == VcpConnect)
                buffer[rx_cnt] = ascii_soh;
            else {
                buffer[rx_cnt] = ascii_eot;
                vcp->rx_buf_len = 0;
            }
            rx_cnt++;
            return rx_cnt;
        }

        if (osSemaphoreAcquire(vcp->rx_sem, timeout) == osErrorTimeout)
            return rx_cnt;
        
        furi_check(osMutexAcquire(vcp->usb_mutex, osWaitForever) == osOK);
        size_t len = furi_hal_cdc_receive(VCP_IF_NUM, vcp->rx_buf, USB_CDC_PKT_LEN);
        furi_check(osMutexRelease(vcp->usb_mutex) == osOK);

        vcp->rx_buf_len = len;
        vcp->rx_buf_start = 0;

        if (vcp->rx_buf_len > (size - rx_cnt)) {
            len = size - rx_cnt;
            memcpy(&buffer[rx_cnt], vcp->rx_buf, len);
            vcp->rx_buf_len -= len;
            vcp->rx_buf_start += len;
        } else {
            memcpy(&buffer[rx_cnt], vcp->rx_buf, vcp->rx_buf_len);
            vcp->rx_buf_len = 0;
        }
        rx_cnt += len;
    }
    return rx_cnt;
}

size_t furi_hal_vcp_rx(uint8_t* buffer, size_t size) {
    furi_assert(vcp);

    return furi_hal_vcp_rx_with_timeout(buffer, size, portMAX_DELAY);
}

void furi_hal_vcp_tx(const uint8_t* buffer, size_t size) {
    furi_assert(vcp);

    while (size > 0 && vcp->connected) {
        furi_check(osSemaphoreAcquire(vcp->tx_sem, osWaitForever) == osOK);
        if (!vcp->connected)
            break;

        size_t batch_size = size;
        if (batch_size > USB_CDC_PKT_LEN) {
            batch_size = USB_CDC_PKT_LEN;
        }

        furi_check(osMutexAcquire(vcp->usb_mutex, osWaitForever) == osOK);
        furi_hal_cdc_send(VCP_IF_NUM, (uint8_t*)buffer, batch_size);
        furi_check(osMutexRelease(vcp->usb_mutex) == osOK);

        size -= batch_size;
        buffer += batch_size;
    }
}

static void vcp_state_callback(uint8_t state) {
    if (state == 1) {
        osSemaphoreRelease(vcp->rx_sem);
        //osSemaphoreRelease(vcp->tx_sem);
    }
    else if (vcp->connected) {
        vcp->connected = false;
        osSemaphoreRelease(vcp->rx_sem);
        VcpEvent evt = VcpDisconnect;
        osMessageQueuePut(vcp->event_queue, &evt, 0, 0);
        //osSemaphoreRelease(vcp->tx_sem);
    }
}

static void vcp_on_cdc_control_line(uint8_t state) {
    // bit 0: DTR state, bit 1: RTS state
    bool dtr = state & 0b1;

    if (dtr) {
        if (!vcp->connected) {
            vcp->connected = true;
            VcpEvent evt = VcpConnect;
            osMessageQueuePut(vcp->event_queue, &evt, 0, 0);
        }
    } else {
        if (vcp->connected) {
            VcpEvent evt = VcpDisconnect;
            osMessageQueuePut(vcp->event_queue, &evt, 0, 0);
            vcp->connected = false;
        }
    }

    osSemaphoreRelease(vcp->tx_sem);
    osSemaphoreRelease(vcp->rx_sem);
}

static void vcp_on_cdc_rx() {
    if (vcp->connected == false)
        return;
    osSemaphoreRelease(vcp->rx_sem);
}

static void vcp_on_cdc_tx_complete() {
    osSemaphoreRelease(vcp->tx_sem);
}

bool furi_hal_vcp_is_connected(void) {
    return vcp->connected;
}

