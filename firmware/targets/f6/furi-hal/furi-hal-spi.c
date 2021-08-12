#include "furi-hal-spi.h"
#include "furi-hal-resources.h"

#include <stdbool.h>
#include <string.h>
#include <furi.h>

#include <stm32wbxx_ll_spi.h>
#include <stm32wbxx_ll_utils.h>
#include <stm32wbxx_ll_cortex.h>

void furi_hal_spi_init() {
    // Spi structure is const, but mutex is not
    // Need some hell-ish casting to make it work
    *(osMutexId_t*)spi_r.mutex = osMutexNew(NULL);
    *(osMutexId_t*)spi_d.mutex = osMutexNew(NULL);
    // 
    for (size_t i=0; i<FuriHalSpiDeviceIdMax; ++i) {
        hal_gpio_write(furi_hal_spi_devices[i].chip_select, true);
        hal_gpio_init(
            furi_hal_spi_devices[i].chip_select,
            GpioModeOutputPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh
        );
    }

    hal_gpio_init_ex(&gpio_spi_r_miso, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedVeryHigh, GpioAltFn5SPI1);
    hal_gpio_init_ex(&gpio_spi_r_mosi, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedVeryHigh, GpioAltFn5SPI1);
    hal_gpio_init_ex(&gpio_spi_r_sck, GpioModeAltFunctionPushPull, GpioPullNo, GpioSpeedVeryHigh, GpioAltFn5SPI1);

    hal_gpio_init_ex(&gpio_spi_d_miso, GpioModeAltFunctionPushPull, GpioPullUp, GpioSpeedVeryHigh, GpioAltFn5SPI2);
    hal_gpio_init_ex(&gpio_spi_d_mosi, GpioModeAltFunctionPushPull, GpioPullUp, GpioSpeedVeryHigh, GpioAltFn5SPI2);
    hal_gpio_init_ex(&gpio_spi_d_sck, GpioModeAltFunctionPushPull, GpioPullUp, GpioSpeedVeryHigh, GpioAltFn5SPI2);

    FURI_LOG_I("FuriHalSpi", "Init OK");
}

void furi_hal_spi_bus_lock(const FuriHalSpiBus* bus) {
    furi_assert(bus);
    furi_check(osMutexAcquire(*bus->mutex, osWaitForever) == osOK);
}

void furi_hal_spi_bus_unlock(const FuriHalSpiBus* bus) {
    furi_assert(bus);
    furi_check(osMutexRelease(*bus->mutex) == osOK);
}

void furi_hal_spi_bus_configure(const FuriHalSpiBus* bus, const LL_SPI_InitTypeDef* config) {
    furi_assert(bus);

    LL_SPI_DeInit((SPI_TypeDef*)bus->spi);
    LL_SPI_Init((SPI_TypeDef*)bus->spi, (LL_SPI_InitTypeDef*)config);
    LL_SPI_SetRxFIFOThreshold((SPI_TypeDef*)bus->spi, LL_SPI_RX_FIFO_TH_QUARTER);
    LL_SPI_Enable((SPI_TypeDef*)bus->spi);
}

void furi_hal_spi_bus_end_txrx(const FuriHalSpiBus* bus, uint32_t timeout) {
    while(LL_SPI_GetTxFIFOLevel((SPI_TypeDef *)bus->spi) != LL_SPI_TX_FIFO_EMPTY);
    while(LL_SPI_IsActiveFlag_BSY((SPI_TypeDef *)bus->spi));
    while(LL_SPI_GetRxFIFOLevel((SPI_TypeDef *)bus->spi) != LL_SPI_RX_FIFO_EMPTY) {
        LL_SPI_ReceiveData8((SPI_TypeDef *)bus->spi);
    }
}

bool furi_hal_spi_bus_rx(const FuriHalSpiBus* bus, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(bus);
    furi_assert(buffer);
    furi_assert(size > 0);

    return furi_hal_spi_bus_trx(bus, buffer, buffer, size, timeout);
}

bool furi_hal_spi_bus_tx(const FuriHalSpiBus* bus, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(bus);
    furi_assert(buffer);
    furi_assert(size > 0);
    bool ret = true;

    while(size > 0) {
        if (LL_SPI_IsActiveFlag_TXE((SPI_TypeDef *)bus->spi)) {
            LL_SPI_TransmitData8((SPI_TypeDef *)bus->spi, *buffer);
            buffer++;
            size--;
        }
    }

    furi_hal_spi_bus_end_txrx(bus, timeout);
    LL_SPI_ClearFlag_OVR((SPI_TypeDef *)bus->spi);

    return ret;
}

bool furi_hal_spi_bus_trx(const FuriHalSpiBus* bus, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size, uint32_t timeout) {
    furi_assert(bus);
    furi_assert(tx_buffer);
    furi_assert(rx_buffer);
    furi_assert(size > 0);

    bool ret = true;
    size_t tx_size = size;
    bool tx_allowed = true;

    while(size > 0) {
        if(tx_size > 0 && LL_SPI_IsActiveFlag_TXE((SPI_TypeDef *)bus->spi) && tx_allowed) {
            LL_SPI_TransmitData8((SPI_TypeDef *)bus->spi, *tx_buffer);
            tx_buffer++;
            tx_size--;
            tx_allowed = false;
        }
        
        if(LL_SPI_IsActiveFlag_RXNE((SPI_TypeDef *)bus->spi)) {
            *rx_buffer = LL_SPI_ReceiveData8((SPI_TypeDef *)bus->spi);
            rx_buffer++;
            size--;
            tx_allowed = true;
        }
    }

    furi_hal_spi_bus_end_txrx(bus, timeout);

    return ret;
}

void furi_hal_spi_device_configure(const FuriHalSpiDevice* device) {
    furi_assert(device);
    furi_assert(device->config);

    furi_hal_spi_bus_configure(device->bus, device->config);
}

const FuriHalSpiDevice* furi_hal_spi_device_get(FuriHalSpiDeviceId device_id) {
    furi_assert(device_id < FuriHalSpiDeviceIdMax);

    const FuriHalSpiDevice* device = &furi_hal_spi_devices[device_id];
    assert(device);
    furi_hal_spi_bus_lock(device->bus);
    furi_hal_spi_device_configure(device);

    return device;
}

void furi_hal_spi_device_return(const FuriHalSpiDevice* device) {
    furi_hal_spi_bus_unlock(device->bus);
}

bool furi_hal_spi_device_rx(const FuriHalSpiDevice* device, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(device);
    furi_assert(buffer);
    furi_assert(size > 0);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, false);
    }

    bool ret = furi_hal_spi_bus_rx(device->bus, buffer, size, timeout);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, true);
    }

    return ret;
}

bool furi_hal_spi_device_tx(const FuriHalSpiDevice* device, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(device);
    furi_assert(buffer);
    furi_assert(size > 0);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, false);
    }

    bool ret = furi_hal_spi_bus_tx(device->bus, buffer, size, timeout);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, true);
    }

    return ret;
}

bool furi_hal_spi_device_trx(const FuriHalSpiDevice* device, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size, uint32_t timeout) {
    furi_assert(device);
    furi_assert(tx_buffer);
    furi_assert(rx_buffer);
    furi_assert(size > 0);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, false);
    }

    bool ret = furi_hal_spi_bus_trx(device->bus, tx_buffer, rx_buffer, size, timeout);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, true);
    }

    return ret;
}
