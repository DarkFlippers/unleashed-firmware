#include "furi-hal-spi.h"
#include <furi-hal-resources.h>
#include <stdbool.h>
#include <string.h>
#include <spi.h>
#include <furi.h>


extern void Enable_SPI(SPI_HandleTypeDef* spi);

void furi_hal_spi_init() {
    // Spi structure is const, but mutex is not
    // Need some hell-ish casting to make it work
    *(osMutexId_t*)spi_r.mutex = osMutexNew(NULL);
    *(osMutexId_t*)spi_d.mutex = osMutexNew(NULL);
    // 
    for (size_t i=0; i<FuriHalSpiDeviceIdMax; ++i) {
        hal_gpio_init(
            furi_hal_spi_devices[i].chip_select,
            GpioModeOutputPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh
        );
    }
    FURI_LOG_I("FuriHalSpi", "Init OK");
}

void furi_hal_spi_bus_lock(const FuriHalSpiBus* bus) {
    furi_assert(bus);
    if (bus->mutex) {
        osMutexAcquire(*bus->mutex, osWaitForever);
    }
}

void furi_hal_spi_bus_unlock(const FuriHalSpiBus* bus) {
    furi_assert(bus);
    if (bus->mutex) {
        osMutexRelease(*bus->mutex);
    }
}

void furi_hal_spi_bus_configure(const FuriHalSpiBus* bus, const SPI_InitTypeDef* config) {
    furi_assert(bus);

    if(memcmp(&bus->spi->Init, config, sizeof(SPI_InitTypeDef))) {
        memcpy((SPI_InitTypeDef*)&bus->spi->Init, config, sizeof(SPI_InitTypeDef));
        if(HAL_SPI_Init((SPI_HandleTypeDef*)bus->spi) != HAL_OK) {
            Error_Handler();
        }
        Enable_SPI((SPI_HandleTypeDef*)bus->spi);
    }
}

void furi_hal_spi_bus_reset(const FuriHalSpiBus* bus) {
    furi_assert(bus);

    HAL_SPI_DeInit((SPI_HandleTypeDef*)bus->spi);
    HAL_SPI_Init((SPI_HandleTypeDef*)bus->spi);
    Enable_SPI((SPI_HandleTypeDef*)bus->spi);
}

bool furi_hal_spi_bus_rx(const FuriHalSpiBus* bus, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(bus);
    furi_assert(buffer);
    furi_assert(size > 0);

    HAL_StatusTypeDef ret = HAL_SPI_Receive((SPI_HandleTypeDef *)bus->spi, buffer, size, HAL_MAX_DELAY);

    return ret == HAL_OK;
}

bool furi_hal_spi_bus_tx(const FuriHalSpiBus* bus, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(bus);
    furi_assert(buffer);
    furi_assert(size > 0);

    HAL_StatusTypeDef ret = HAL_SPI_Transmit((SPI_HandleTypeDef *)bus->spi, buffer, size, HAL_MAX_DELAY);

    return ret == HAL_OK;
}

bool furi_hal_spi_bus_trx(const FuriHalSpiBus* bus, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size, uint32_t timeout) {
    furi_assert(bus);
    furi_assert(tx_buffer);
    furi_assert(rx_buffer);
    furi_assert(size > 0);

    HAL_StatusTypeDef ret = HAL_SPI_TransmitReceive((SPI_HandleTypeDef *)bus->spi, tx_buffer, rx_buffer, size, HAL_MAX_DELAY);

    return ret == HAL_OK;
}

const FuriHalSpiDevice* furi_hal_spi_device_get(FuriHalSpiDeviceId device_id) {
    furi_assert(device_id < FuriHalSpiDeviceIdMax);

    const FuriHalSpiDevice* device = &furi_hal_spi_devices[device_id];
    assert(device);
    furi_hal_spi_bus_lock(device->bus);

    if (device->config) {
        memcpy((SPI_InitTypeDef*)&device->bus->spi->Init, device->config, sizeof(SPI_InitTypeDef));
        if(HAL_SPI_Init((SPI_HandleTypeDef *)device->bus->spi) != HAL_OK) {
            Error_Handler();
        }
        Enable_SPI((SPI_HandleTypeDef *)device->bus->spi);
    }

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

    bool ret = furi_hal_spi_bus_rx(device->bus, buffer, size, HAL_MAX_DELAY);

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

    bool ret = furi_hal_spi_bus_tx(device->bus, buffer, size, HAL_MAX_DELAY);

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

    bool ret = furi_hal_spi_bus_trx(device->bus, tx_buffer, rx_buffer, size, HAL_MAX_DELAY);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, true);
    }

    return ret;
}
