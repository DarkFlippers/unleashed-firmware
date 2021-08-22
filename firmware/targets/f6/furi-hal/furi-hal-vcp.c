#include <furi-hal-vcp_i.h>

#include <furi.h>
#include <usbd_cdc_if.h>
#include <stream_buffer.h>

#define FURI_HAL_VCP_RX_BUFFER_SIZE (APP_RX_DATA_SIZE * 5)

extern USBD_HandleTypeDef hUsbDeviceFS;

typedef struct {
    volatile bool connected;

    StreamBufferHandle_t rx_stream;
    volatile bool rx_stream_full;

    osSemaphoreId_t tx_semaphore;
} FuriHalVcp;

static FuriHalVcp* furi_hal_vcp = NULL;

static const uint8_t ascii_soh = 0x01;
static const uint8_t ascii_eot = 0x04;

void furi_hal_vcp_init() {
    furi_hal_vcp = furi_alloc(sizeof(FuriHalVcp));
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
        &&xStreamBufferSpacesAvailable(furi_hal_vcp->rx_stream) >= APP_RX_DATA_SIZE) {
        furi_hal_vcp->rx_stream_full = false;
        // data accepted, start waiting for next packet
        USBD_CDC_ReceivePacket(&hUsbDeviceFS);
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

        if (CDC_Transmit_FS((uint8_t*)buffer, batch_size) == USBD_OK) {
            size -= batch_size;
            buffer += batch_size;
        } else {
            FURI_LOG_E("FuriHalVcp", "CDC_Transmit_FS failed");
            osDelay(50);
        }
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
    // bit 0: DTR state, bit 1: RTS state
    // bool dtr = state & 0b01;
    bool dtr = state & 0b1;

    if (dtr) {
        if (!furi_hal_vcp->connected) {
            furi_hal_vcp->connected = true;
            furi_hal_vcp_on_cdc_rx(&ascii_soh, 1); // SOH
        }
    } else {
        if (furi_hal_vcp->connected) {
            furi_hal_vcp_on_cdc_rx(&ascii_eot, 1); // EOT
            furi_hal_vcp->connected = false;
        }
    }

    osSemaphoreRelease(furi_hal_vcp->tx_semaphore);
}

void furi_hal_vcp_on_cdc_rx(const uint8_t* buffer, size_t size) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    size_t ret = xStreamBufferSendFromISR(furi_hal_vcp->rx_stream, buffer, size, &xHigherPriorityTaskWoken);
    furi_check(ret == size);
    
    if (xStreamBufferSpacesAvailable(furi_hal_vcp->rx_stream) >= APP_RX_DATA_SIZE) {
        USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    } else {
        furi_hal_vcp->rx_stream_full = true;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void furi_hal_vcp_on_cdc_tx_complete(size_t size) {
    osSemaphoreRelease(furi_hal_vcp->tx_semaphore);
}

