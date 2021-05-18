#include "api-hal-spi.h"
#include <api-hal-resources.h>
#include <stdbool.h>
#include <string.h>
#include <spi.h>
#include <furi.h>


extern void Enable_SPI(SPI_HandleTypeDef* spi);

void api_hal_spi_init() {
    // Spi structure is const, but mutex is not
    // Need some hell-ish casting to make it work
    *(osMutexId_t*)spi_r.mutex = osMutexNew(NULL);
    *(osMutexId_t*)spi_d.mutex = osMutexNew(NULL);
    // 
    for (size_t i=0; i<ApiHalSpiDeviceIdMax; ++i) {
        hal_gpio_init(
            api_hal_spi_devices[i].chip_select,
            GpioModeOutputPushPull,
            GpioPullNo,
            GpioSpeedVeryHigh
        );
    }
}

void api_hal_spi_bus_lock(const ApiHalSpiBus* bus) {
    furi_assert(bus);
    if (bus->mutex) {
        osMutexAcquire(*bus->mutex, osWaitForever);
    }
}

void api_hal_spi_bus_unlock(const ApiHalSpiBus* bus) {
    furi_assert(bus);
    if (bus->mutex) {
        osMutexRelease(*bus->mutex);
    }
}

bool api_hal_spi_bus_rx(const ApiHalSpiBus* bus, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(bus);
    furi_assert(buffer);
    furi_assert(size > 0);

    HAL_StatusTypeDef ret = HAL_SPI_Receive((SPI_HandleTypeDef *)bus->spi, buffer, size, HAL_MAX_DELAY);

    return ret == HAL_OK;
}

bool api_hal_spi_bus_tx(const ApiHalSpiBus* bus, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(bus);
    furi_assert(buffer);
    furi_assert(size > 0);

    HAL_StatusTypeDef ret = HAL_SPI_Transmit((SPI_HandleTypeDef *)bus->spi, buffer, size, HAL_MAX_DELAY);

    return ret == HAL_OK;
}

bool api_hal_spi_bus_trx(const ApiHalSpiBus* bus, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size, uint32_t timeout) {
    furi_assert(bus);
    furi_assert(tx_buffer);
    furi_assert(rx_buffer);
    furi_assert(size > 0);

    HAL_StatusTypeDef ret = HAL_SPI_TransmitReceive((SPI_HandleTypeDef *)bus->spi, tx_buffer, rx_buffer, size, HAL_MAX_DELAY);

    return ret == HAL_OK;
}

const ApiHalSpiDevice* api_hal_spi_device_get(ApiHalSpiDeviceId device_id) {
    furi_assert(device_id < ApiHalSpiDeviceIdMax);

    const ApiHalSpiDevice* device = &api_hal_spi_devices[device_id];
    assert(device);
    api_hal_spi_bus_lock(device->bus);

    if (device->config) {
        memcpy((SPI_InitTypeDef*)&device->bus->spi->Init, device->config, sizeof(SPI_InitTypeDef));
        if(HAL_SPI_Init((SPI_HandleTypeDef *)device->bus->spi) != HAL_OK) {
            Error_Handler();
        }
        Enable_SPI((SPI_HandleTypeDef *)device->bus->spi);
    }

    return device;
}

void api_hal_spi_device_return(const ApiHalSpiDevice* device) {
    api_hal_spi_bus_unlock(device->bus);
}

bool api_hal_spi_device_rx(const ApiHalSpiDevice* device, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(device);
    furi_assert(buffer);
    furi_assert(size > 0);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, false);
    }

    bool ret = api_hal_spi_bus_rx(device->bus, buffer, size, HAL_MAX_DELAY);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, true);
    }

    return ret;
}

bool api_hal_spi_device_tx(const ApiHalSpiDevice* device, uint8_t* buffer, size_t size, uint32_t timeout) {
    furi_assert(device);
    furi_assert(buffer);
    furi_assert(size > 0);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, false);
    }

    bool ret = api_hal_spi_bus_tx(device->bus, buffer, size, HAL_MAX_DELAY);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, true);
    }

    return ret;
}

bool api_hal_spi_device_trx(const ApiHalSpiDevice* device, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size, uint32_t timeout) {
    furi_assert(device);
    furi_assert(tx_buffer);
    furi_assert(rx_buffer);
    furi_assert(size > 0);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, false);
    }

    bool ret = api_hal_spi_bus_trx(device->bus, tx_buffer, rx_buffer, size, HAL_MAX_DELAY);

    if (device->chip_select) {
        hal_gpio_write(device->chip_select, true);
    }

    return ret;
}

void api_hal_spi_apply_config(const SPIDevice* device) {
    osKernelLock();

    memcpy((SPI_InitTypeDef*)&device->bus->spi->Init, &device->config, sizeof(SPI_InitTypeDef));

    if(HAL_SPI_Init((SPI_HandleTypeDef*)device->bus->spi) != HAL_OK) {
        Error_Handler();
    }

    Enable_SPI((SPI_HandleTypeDef*)device->bus->spi);

    osKernelUnlock();
}

bool api_hal_spi_config_are_actual(const SPIDevice* device) {
    return (memcmp(&device->config, &device->bus->spi->Init, sizeof(SPI_InitTypeDef)) == 0);
}

void api_hal_spi_config_device(const SPIDevice* device) {
    if(!api_hal_spi_config_are_actual(device)) {
        api_hal_spi_apply_config(device);
    }
}

void api_hal_spi_lock_device(const SPIDevice* device) {
    api_hal_spi_bus_lock(device->bus);
    api_hal_spi_config_device(device);
}

void api_hal_spi_unlock_device(const SPIDevice* device) {
    api_hal_spi_bus_unlock(device->bus);
}
