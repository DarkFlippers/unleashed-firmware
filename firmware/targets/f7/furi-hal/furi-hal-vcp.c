#include <furi-hal-vcp_i.h>
#include <furi-hal-usb-cdc_i.h>

#include <furi.h>
#include <stream_buffer.h>

#define APP_RX_DATA_SIZE CDC_DATA_SZ
#define APP_TX_DATA_SIZE CDC_DATA_SZ
#define FURI_HAL_VCP_RX_BUFFER_SIZE (APP_RX_DATA_SIZE * 16)

typedef struct {
    volatile bool connected;

    StreamBufferHandle_t rx_stream;
    volatile bool rx_stream_full;

    osSemaphoreId_t tx_semaphore;
} FuriHalVcp;

static FuriHalVcp* furi_hal_vcp = NULL;

static const uint8_t ascii_soh = 0x01;
static const uint8_t ascii_eot = 0x04;

static uint8_t* vcp_rx_buf;

void furi_hal_vcp_init() {
    furi_hal_vcp = furi_alloc(sizeof(FuriHalVcp));
    vcp_rx_buf = furi_alloc(APP_RX_DATA_SIZE);
    furi_hal_vcp->connected = false;
    
    furi_hal_vcp->rx_stream = xStreamBufferCreate(FURI_HAL_VCP_RX_BUFFER_SIZE, 1);
    furi_hal_vcp->rx_stream_full = false;

    furi_hal_vcp->tx_semaphore = osSemaphoreNew(1, 1, NULL);

    FURI_LOG_I("FuriHalVcp", "Init OK");
}

size_t furi_hal_vcp_rx(uint8_t* buffer, size_t size) {
    furi_assert(furi_hal_vcp);

    size_t received = xStreamBufferReceive(furi_hal_vcp->rx_stream, buffer, size, portMAX_DELAY);

    if(furi_hal_vcp->rx_stream_full
        && xStreamBufferSpacesAvailable(furi_hal_vcp->rx_stream) >= APP_RX_DATA_SIZE) {
        furi_hal_vcp->rx_stream_full = false;
    }

    return received;
}

size_t furi_hal_vcp_rx_with_timeout(uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(furi_hal_vcp);
    return xStreamBufferReceive(furi_hal_vcp->rx_stream, buffer, size, timeout);
}

void furi_hal_vcp_tx(const uint8_t* buffer, size_t size) {
    furi_assert(furi_hal_vcp);

    while (size > 0 && furi_hal_vcp->connected) {
        furi_check(osSemaphoreAcquire(furi_hal_vcp->tx_semaphore, osWaitForever) == osOK);
        if (!furi_hal_vcp->connected)
            break;

        size_t batch_size = size;
        if (batch_size > APP_TX_DATA_SIZE) {
            batch_size = APP_TX_DATA_SIZE;
        }

        furi_hal_cdc_send(0, (uint8_t*)buffer, batch_size);
        size -= batch_size;
        buffer += batch_size;
    }
}

void furi_hal_vcp_on_usb_resume() {
    osSemaphoreRelease(furi_hal_vcp->tx_semaphore);
}

void furi_hal_vcp_on_usb_suspend() {
    if (furi_hal_vcp->connected) {
        furi_hal_vcp->connected = false;
        osSemaphoreRelease(furi_hal_vcp->tx_semaphore);
    }
}

void furi_hal_vcp_on_cdc_control_line(uint8_t state) {

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // bit 0: DTR state, bit 1: RTS state
    // bool dtr = state & 0b01;
    bool dtr = state & 0b1;

    if (dtr) {
        if (!furi_hal_vcp->connected) {
            furi_hal_vcp->connected = true;
            xStreamBufferSendFromISR(furi_hal_vcp->rx_stream, &ascii_soh, 1, &xHigherPriorityTaskWoken); // SOH 

        }
    } else {
        if (furi_hal_vcp->connected) {
            xStreamBufferSendFromISR(furi_hal_vcp->rx_stream, &ascii_eot, 1, &xHigherPriorityTaskWoken); // EOT
            furi_hal_vcp->connected = false;
        }
    }

    osSemaphoreRelease(furi_hal_vcp->tx_semaphore);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void furi_hal_vcp_on_cdc_rx(uint8_t if_num) { 
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (if_num == 0) {
        uint16_t max_len = xStreamBufferSpacesAvailable(furi_hal_vcp->rx_stream);
        if (max_len > 0) {
            if (max_len > APP_RX_DATA_SIZE)
                max_len = APP_RX_DATA_SIZE;
            int32_t size = furi_hal_cdc_receive(0, vcp_rx_buf, max_len);

            if (size > 0) {
                size_t ret = xStreamBufferSendFromISR(furi_hal_vcp->rx_stream, vcp_rx_buf, size, &xHigherPriorityTaskWoken);
                furi_check(ret == size);
            }
        } else {
            furi_hal_vcp->rx_stream_full = true;
        };
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void furi_hal_vcp_on_cdc_tx_complete(uint8_t if_num) {
    if (if_num == 0)
        osSemaphoreRelease(furi_hal_vcp->tx_semaphore);
}

