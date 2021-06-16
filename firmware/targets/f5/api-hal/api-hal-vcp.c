#include <api-hal-vcp.h>
#include <usbd_cdc_if.h>
#include <furi.h>
#include <stream_buffer.h>

#define API_HAL_VCP_RX_BUFFER_SIZE 600

typedef struct {
    StreamBufferHandle_t rx_stream;
    osSemaphoreId_t tx_semaphore;
    volatile bool alive;
    volatile bool underrun;
} ApiHalVcp;

static ApiHalVcp* api_hal_vcp = NULL;

static const uint8_t ascii_soh = 0x01;
static const uint8_t ascii_eot = 0x04;

void _api_hal_vcp_init();
void _api_hal_vcp_deinit();
void _api_hal_vcp_control_line(uint8_t state);
void _api_hal_vcp_rx_callback(const uint8_t* buffer, size_t size);
void _api_hal_vcp_tx_complete(size_t size);

void api_hal_vcp_init() {
    api_hal_vcp = furi_alloc(sizeof(ApiHalVcp));
    api_hal_vcp->rx_stream = xStreamBufferCreate(API_HAL_VCP_RX_BUFFER_SIZE, 1);
    api_hal_vcp->tx_semaphore = osSemaphoreNew(1, 1, NULL);
    api_hal_vcp->alive = false;
    api_hal_vcp->underrun = false;
}

void _api_hal_vcp_init() {
    osSemaphoreRelease(api_hal_vcp->tx_semaphore);
}

void _api_hal_vcp_deinit() {
    api_hal_vcp->alive = false;
    osSemaphoreRelease(api_hal_vcp->tx_semaphore);
}

void _api_hal_vcp_control_line(uint8_t state) {
    // bit 0: DTR state, bit 1: RTS state
    // bool dtr = state & 0b01;
    bool dtr = state & 0b1;

    if (dtr) {
        if (!api_hal_vcp->alive) {
            api_hal_vcp->alive = true;
            _api_hal_vcp_rx_callback(&ascii_soh, 1); // SOH
        }
    } else {
        if (api_hal_vcp->alive) {
            _api_hal_vcp_rx_callback(&ascii_eot, 1); // EOT
            api_hal_vcp->alive = false;
        }
    }

    osSemaphoreRelease(api_hal_vcp->tx_semaphore);
}

void _api_hal_vcp_rx_callback(const uint8_t* buffer, size_t size) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    size_t ret = xStreamBufferSendFromISR(api_hal_vcp->rx_stream, buffer, size, &xHigherPriorityTaskWoken);
    if (ret != size) {
        api_hal_vcp->underrun = true;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void _api_hal_vcp_tx_complete(size_t size) {
    osSemaphoreRelease(api_hal_vcp->tx_semaphore);
}

size_t api_hal_vcp_rx(uint8_t* buffer, size_t size) {
    furi_assert(api_hal_vcp);
    return xStreamBufferReceive(api_hal_vcp->rx_stream, buffer, size, portMAX_DELAY);
}

size_t api_hal_vcp_rx_with_timeout(uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(api_hal_vcp);
    return xStreamBufferReceive(api_hal_vcp->rx_stream, buffer, size, timeout);
}

void api_hal_vcp_tx(const uint8_t* buffer, size_t size) {
    furi_assert(api_hal_vcp);

    while (size > 0 && api_hal_vcp->alive) {
        furi_check(osSemaphoreAcquire(api_hal_vcp->tx_semaphore, osWaitForever) == osOK);

        size_t batch_size = size;
        if (batch_size > APP_TX_DATA_SIZE) {
            batch_size = APP_TX_DATA_SIZE;
        }

        if (CDC_Transmit_FS((uint8_t*)buffer, batch_size) == USBD_OK) {
            size -= batch_size;
            buffer += batch_size;
        } else {
            // Shouldn't be there 
            osDelay(100);
        }
    }
}
