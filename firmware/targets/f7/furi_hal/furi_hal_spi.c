#include <furi_hal_spi.h>
#include <furi_hal_resources.h>
#include <furi_hal_power.h>

#include <stdbool.h>
#include <string.h>

#include <stm32wbxx_ll_spi.h>
#include <stm32wbxx_ll_utils.h>
#include <stm32wbxx_ll_cortex.h>

void furi_hal_spi_bus_init(FuriHalSpiBus* bus) {
    furi_assert(bus);
    bus->callback(bus, FuriHalSpiBusEventInit);
}

void furi_hal_spi_bus_deinit(FuriHalSpiBus* bus) {
    furi_assert(bus);
    bus->callback(bus, FuriHalSpiBusEventDeinit);
}

void furi_hal_spi_bus_handle_init(FuriHalSpiBusHandle* handle) {
    furi_assert(handle);
    handle->callback(handle, FuriHalSpiBusHandleEventInit);
}

void furi_hal_spi_bus_handle_deinit(FuriHalSpiBusHandle* handle) {
    furi_assert(handle);
    handle->callback(handle, FuriHalSpiBusHandleEventDeinit);
}

void furi_hal_spi_acquire(FuriHalSpiBusHandle* handle) {
    furi_assert(handle);

    furi_hal_power_insomnia_enter();

    handle->bus->callback(handle->bus, FuriHalSpiBusEventLock);
    handle->bus->callback(handle->bus, FuriHalSpiBusEventActivate);

    furi_assert(handle->bus->current_handle == NULL);

    handle->bus->current_handle = handle;
    handle->callback(handle, FuriHalSpiBusHandleEventActivate);
}

void furi_hal_spi_release(FuriHalSpiBusHandle* handle) {
    furi_assert(handle);
    furi_assert(handle->bus->current_handle == handle);

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
    furi_assert(handle);
    furi_assert(handle->bus->current_handle == handle);
    furi_assert(buffer);
    furi_assert(size > 0);

    return furi_hal_spi_bus_trx(handle, buffer, buffer, size, timeout);
}

bool furi_hal_spi_bus_tx(
    FuriHalSpiBusHandle* handle,
    uint8_t* buffer,
    size_t size,
    uint32_t timeout) {
    furi_assert(handle);
    furi_assert(handle->bus->current_handle == handle);
    furi_assert(buffer);
    furi_assert(size > 0);
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
    uint8_t* tx_buffer,
    uint8_t* rx_buffer,
    size_t size,
    uint32_t timeout) {
    furi_assert(handle);
    furi_assert(handle->bus->current_handle == handle);
    furi_assert(size > 0);

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
