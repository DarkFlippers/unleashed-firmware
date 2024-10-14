#include <furi.h>
#include <furi_hal_spi.h>
#include <furi_hal_resources.h>
#include <furi_hal_power.h>
#include <furi_hal_interrupt.h>

#include <stm32wbxx_ll_dma.h>
#include <stm32wbxx_ll_spi.h>
#include <stm32wbxx_ll_utils.h>
#include <stm32wbxx_ll_cortex.h>

#define TAG "FuriHalSpi"

#define SPI_DMA            DMA2
#define SPI_DMA_RX_CHANNEL LL_DMA_CHANNEL_6
#define SPI_DMA_TX_CHANNEL LL_DMA_CHANNEL_7
#define SPI_DMA_RX_IRQ     FuriHalInterruptIdDma2Ch6
#define SPI_DMA_TX_IRQ     FuriHalInterruptIdDma2Ch7
#define SPI_DMA_RX_DEF     SPI_DMA, SPI_DMA_RX_CHANNEL
#define SPI_DMA_TX_DEF     SPI_DMA, SPI_DMA_TX_CHANNEL

// For simplicity, I assume that only one SPI DMA transaction can occur at a time.
static FuriSemaphore* spi_dma_lock = NULL;
static FuriSemaphore* spi_dma_completed = NULL;

void furi_hal_spi_dma_init(void) {
    spi_dma_lock = furi_semaphore_alloc(1, 1);
    spi_dma_completed = furi_semaphore_alloc(1, 1);
}

void furi_hal_spi_bus_init(FuriHalSpiBus* bus) {
    furi_check(bus);
    bus->callback(bus, FuriHalSpiBusEventInit);
}

void furi_hal_spi_bus_deinit(FuriHalSpiBus* bus) {
    furi_check(bus);
    bus->callback(bus, FuriHalSpiBusEventDeinit);
}

void furi_hal_spi_bus_handle_init(FuriHalSpiBusHandle* handle) {
    furi_check(handle);
    handle->callback(handle, FuriHalSpiBusHandleEventInit);
}

void furi_hal_spi_bus_handle_deinit(FuriHalSpiBusHandle* handle) {
    furi_check(handle);
    handle->callback(handle, FuriHalSpiBusHandleEventDeinit);
}

void furi_hal_spi_acquire(FuriHalSpiBusHandle* handle) {
    furi_check(handle);

    furi_hal_power_insomnia_enter();

    handle->bus->callback(handle->bus, FuriHalSpiBusEventLock);
    handle->bus->callback(handle->bus, FuriHalSpiBusEventActivate);

    furi_check(handle->bus->current_handle == NULL);

    handle->bus->current_handle = handle;
    handle->callback(handle, FuriHalSpiBusHandleEventActivate);
}

void furi_hal_spi_release(FuriHalSpiBusHandle* handle) {
    furi_check(handle);
    furi_check(handle->bus->current_handle == handle);

    // Handle event and unset handle
    handle->callback(handle, FuriHalSpiBusHandleEventDeactivate);
    handle->bus->current_handle = NULL;

    // Bus events
    handle->bus->callback(handle->bus, FuriHalSpiBusEventDeactivate);
    handle->bus->callback(handle->bus, FuriHalSpiBusEventUnlock);

    furi_hal_power_insomnia_exit();
}

static void furi_hal_spi_bus_end_txrx(FuriHalSpiBusHandle* handle, uint32_t timeout) {
    UNUSED(timeout); // FIXME
    while(LL_SPI_GetTxFIFOLevel(handle->bus->spi) != LL_SPI_TX_FIFO_EMPTY)
        ;
    while(LL_SPI_IsActiveFlag_BSY(handle->bus->spi))
        ;
    while(LL_SPI_GetRxFIFOLevel(handle->bus->spi) != LL_SPI_RX_FIFO_EMPTY) {
        LL_SPI_ReceiveData8(handle->bus->spi);
    }
}

bool furi_hal_spi_bus_rx(
    FuriHalSpiBusHandle* handle,
    uint8_t* buffer,
    size_t size,
    uint32_t timeout) {
    furi_check(handle);
    furi_check(handle->bus->current_handle == handle);
    furi_check(buffer);
    furi_check(size > 0);

    return furi_hal_spi_bus_trx(handle, buffer, buffer, size, timeout);
}

bool furi_hal_spi_bus_tx(
    FuriHalSpiBusHandle* handle,
    const uint8_t* buffer,
    size_t size,
    uint32_t timeout) {
    furi_check(handle);
    furi_check(handle->bus->current_handle == handle);
    furi_check(buffer);
    furi_check(size > 0);

    bool ret = true;

    while(size > 0) {
        if(LL_SPI_IsActiveFlag_TXE(handle->bus->spi)) {
            LL_SPI_TransmitData8(handle->bus->spi, *buffer);
            buffer++;
            size--;
        }
    }

    furi_hal_spi_bus_end_txrx(handle, timeout);
    LL_SPI_ClearFlag_OVR(handle->bus->spi);

    return ret;
}

bool furi_hal_spi_bus_trx(
    FuriHalSpiBusHandle* handle,
    const uint8_t* tx_buffer,
    uint8_t* rx_buffer,
    size_t size,
    uint32_t timeout) {
    furi_check(handle);
    furi_check(handle->bus->current_handle == handle);
    furi_check(size > 0);

    bool ret = true;
    size_t tx_size = size;
    bool tx_allowed = true;

    while(size > 0) {
        if(tx_size > 0 && LL_SPI_IsActiveFlag_TXE(handle->bus->spi) && tx_allowed) {
            if(tx_buffer) {
                LL_SPI_TransmitData8(handle->bus->spi, *tx_buffer);
                tx_buffer++;
            } else {
                LL_SPI_TransmitData8(handle->bus->spi, 0xFF);
            }
            tx_size--;
            tx_allowed = false;
        }

        if(LL_SPI_IsActiveFlag_RXNE(handle->bus->spi)) {
            if(rx_buffer) {
                *rx_buffer = LL_SPI_ReceiveData8(handle->bus->spi);
                rx_buffer++;
            } else {
                LL_SPI_ReceiveData8(handle->bus->spi);
            }
            size--;
            tx_allowed = true;
        }
    }

    furi_hal_spi_bus_end_txrx(handle, timeout);

    return ret;
}

static void spi_dma_isr(void* context) {
    UNUSED(context);
#if SPI_DMA_RX_CHANNEL == LL_DMA_CHANNEL_6
    if(LL_DMA_IsActiveFlag_TC6(SPI_DMA) && LL_DMA_IsEnabledIT_TC(SPI_DMA_RX_DEF)) {
        LL_DMA_ClearFlag_TC6(SPI_DMA);
        furi_check(furi_semaphore_release(spi_dma_completed) == FuriStatusOk);
    }
#else
#error Update this code. Would you kindly?
#endif

#if SPI_DMA_TX_CHANNEL == LL_DMA_CHANNEL_7
    if(LL_DMA_IsActiveFlag_TC7(SPI_DMA) && LL_DMA_IsEnabledIT_TC(SPI_DMA_TX_DEF)) {
        LL_DMA_ClearFlag_TC7(SPI_DMA);
        furi_check(furi_semaphore_release(spi_dma_completed) == FuriStatusOk);
    }
#else
#error Update this code. Would you kindly?
#endif
}

bool furi_hal_spi_bus_trx_dma(
    FuriHalSpiBusHandle* handle,
    uint8_t* tx_buffer,
    uint8_t* rx_buffer,
    size_t size,
    uint32_t timeout_ms) {
    furi_check(handle);
    furi_check(handle->bus->current_handle == handle);
    furi_check(size > 0);

    // If scheduler is not running, use blocking mode
    if(!furi_kernel_is_running()) {
        return furi_hal_spi_bus_trx(handle, tx_buffer, rx_buffer, size, timeout_ms);
    }

    // Lock DMA
    furi_check(furi_semaphore_acquire(spi_dma_lock, FuriWaitForever) == FuriStatusOk);

    const uint32_t dma_dummy_u32 = 0xFFFFFFFF;

    bool ret = true;
    SPI_TypeDef* spi = handle->bus->spi;
    uint32_t dma_rx_req;
    uint32_t dma_tx_req;

    if(spi == SPI1) {
        dma_rx_req = LL_DMAMUX_REQ_SPI1_RX;
        dma_tx_req = LL_DMAMUX_REQ_SPI1_TX;
    } else if(spi == SPI2) {
        dma_rx_req = LL_DMAMUX_REQ_SPI2_RX;
        dma_tx_req = LL_DMAMUX_REQ_SPI2_TX;
    } else {
        furi_crash();
    }

    if(rx_buffer == NULL) {
        // Only TX mode, do not use RX channel

        LL_DMA_InitTypeDef dma_config = {0};
        dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (spi->DR);
        dma_config.MemoryOrM2MDstAddress = (uint32_t)tx_buffer;
        dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
        dma_config.Mode = LL_DMA_MODE_NORMAL;
        dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
        dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
        dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
        dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
        dma_config.NbData = size;
        dma_config.PeriphRequest = dma_tx_req;
        dma_config.Priority = LL_DMA_PRIORITY_MEDIUM;
        LL_DMA_Init(SPI_DMA_TX_DEF, &dma_config);

#if SPI_DMA_TX_CHANNEL == LL_DMA_CHANNEL_7
        LL_DMA_ClearFlag_TC7(SPI_DMA);
#else
#error Update this code. Would you kindly?
#endif

        furi_hal_interrupt_set_isr(SPI_DMA_TX_IRQ, spi_dma_isr, NULL);

        bool dma_tx_was_enabled = LL_SPI_IsEnabledDMAReq_TX(spi);
        if(!dma_tx_was_enabled) {
            LL_SPI_EnableDMAReq_TX(spi);
        }

        // acquire semaphore before enabling DMA
        furi_check(furi_semaphore_acquire(spi_dma_completed, timeout_ms) == FuriStatusOk);

        LL_DMA_EnableIT_TC(SPI_DMA_TX_DEF);
        LL_DMA_EnableChannel(SPI_DMA_TX_DEF);

        // and wait for it to be released (DMA transfer complete)
        if(furi_semaphore_acquire(spi_dma_completed, timeout_ms) != FuriStatusOk) {
            ret = false;
            FURI_LOG_E(TAG, "DMA timeout\r\n");
        }
        // release semaphore, because we are using it as a flag
        furi_semaphore_release(spi_dma_completed);

        LL_DMA_DisableIT_TC(SPI_DMA_TX_DEF);
        LL_DMA_DisableChannel(SPI_DMA_TX_DEF);
        if(!dma_tx_was_enabled) {
            LL_SPI_DisableDMAReq_TX(spi);
        }
        furi_hal_interrupt_set_isr(SPI_DMA_TX_IRQ, NULL, NULL);

        LL_DMA_DeInit(SPI_DMA_TX_DEF);
    } else {
        // TRX or RX mode, use both channels
        uint32_t tx_mem_increase_mode;

        if(tx_buffer == NULL) {
            // RX mode, use dummy data instead of TX buffer
            tx_buffer = (uint8_t*)&dma_dummy_u32;
            tx_mem_increase_mode = LL_DMA_MEMORY_NOINCREMENT;
        } else {
            tx_mem_increase_mode = LL_DMA_MEMORY_INCREMENT;
        }

        LL_DMA_InitTypeDef dma_config = {0};
        dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (spi->DR);
        dma_config.MemoryOrM2MDstAddress = (uint32_t)tx_buffer;
        dma_config.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
        dma_config.Mode = LL_DMA_MODE_NORMAL;
        dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
        dma_config.MemoryOrM2MDstIncMode = tx_mem_increase_mode;
        dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
        dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
        dma_config.NbData = size;
        dma_config.PeriphRequest = dma_tx_req;
        dma_config.Priority = LL_DMA_PRIORITY_MEDIUM;
        LL_DMA_Init(SPI_DMA_TX_DEF, &dma_config);

        dma_config.PeriphOrM2MSrcAddress = (uint32_t) & (spi->DR);
        dma_config.MemoryOrM2MDstAddress = (uint32_t)rx_buffer;
        dma_config.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
        dma_config.Mode = LL_DMA_MODE_NORMAL;
        dma_config.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
        dma_config.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
        dma_config.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
        dma_config.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
        dma_config.NbData = size;
        dma_config.PeriphRequest = dma_rx_req;
        dma_config.Priority = LL_DMA_PRIORITY_MEDIUM;
        LL_DMA_Init(SPI_DMA_RX_DEF, &dma_config);

#if SPI_DMA_RX_CHANNEL == LL_DMA_CHANNEL_6
        LL_DMA_ClearFlag_TC6(SPI_DMA);
#else
#error Update this code. Would you kindly?
#endif

        furi_hal_interrupt_set_isr(SPI_DMA_RX_IRQ, spi_dma_isr, NULL);

        bool dma_tx_was_enabled = LL_SPI_IsEnabledDMAReq_TX(spi);
        bool dma_rx_was_enabled = LL_SPI_IsEnabledDMAReq_RX(spi);

        if(!dma_tx_was_enabled) {
            LL_SPI_EnableDMAReq_TX(spi);
        }

        if(!dma_rx_was_enabled) {
            LL_SPI_EnableDMAReq_RX(spi);
        }

        // acquire semaphore before enabling DMA
        furi_check(furi_semaphore_acquire(spi_dma_completed, timeout_ms) == FuriStatusOk);

        LL_DMA_EnableIT_TC(SPI_DMA_RX_DEF);
        LL_DMA_EnableChannel(SPI_DMA_RX_DEF);
        LL_DMA_EnableChannel(SPI_DMA_TX_DEF);

        // and wait for it to be released (DMA transfer complete)
        if(furi_semaphore_acquire(spi_dma_completed, timeout_ms) != FuriStatusOk) {
            ret = false;
            FURI_LOG_E(TAG, "DMA timeout\r\n");
        }
        // release semaphore, because we are using it as a flag
        furi_semaphore_release(spi_dma_completed);

        LL_DMA_DisableIT_TC(SPI_DMA_RX_DEF);

        LL_DMA_DisableChannel(SPI_DMA_TX_DEF);
        LL_DMA_DisableChannel(SPI_DMA_RX_DEF);

        if(!dma_tx_was_enabled) {
            LL_SPI_DisableDMAReq_TX(spi);
        }

        if(!dma_rx_was_enabled) {
            LL_SPI_DisableDMAReq_RX(spi);
        }

        furi_hal_interrupt_set_isr(SPI_DMA_RX_IRQ, NULL, NULL);

        LL_DMA_DeInit(SPI_DMA_TX_DEF);
        LL_DMA_DeInit(SPI_DMA_RX_DEF);
    }

    furi_hal_spi_bus_end_txrx(handle, timeout_ms);

    furi_check(furi_semaphore_release(spi_dma_lock) == FuriStatusOk);

    return ret;
}
