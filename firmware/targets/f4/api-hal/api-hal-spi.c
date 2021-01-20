#include "api-hal-spi.h"
#include <cmsis_os2.h>
#include <stdbool.h>
#include <string.h>

osMutexId_t spi_mutex_r;
osMutexId_t spi_mutex_d;

extern SPI_HandleTypeDef SPI_R;
extern SPI_HandleTypeDef SPI_D;
extern void Enable_SPI(SPI_HandleTypeDef* spi);

void api_hal_spi_init() {
    spi_mutex_r = osMutexNew(NULL);
    spi_mutex_d = osMutexNew(NULL);
}

void api_hal_spi_apply_config(const SPIDevice* device) {
    osKernelLock();

    memcpy(&device->spi->Init, &device->config, sizeof(SPI_InitTypeDef));

    if(HAL_SPI_Init(device->spi) != HAL_OK) {
        Error_Handler();
    }

    Enable_SPI(device->spi);

    osKernelUnlock();
}

bool api_hal_spi_config_are_actual(const SPIDevice* device) {
    return (memcmp(&device->config, &device->spi->Init, sizeof(SPI_InitTypeDef)) == 0);
}

void api_hal_spi_config_device(const SPIDevice* device) {
    if(!api_hal_spi_config_are_actual(device)) {
        api_hal_spi_apply_config(device);
    }
}

void api_hal_spi_lock(SPI_HandleTypeDef* spi) {
    if(spi == &SPI_D) {
        osMutexAcquire(spi_mutex_d, osWaitForever);
    } else if(spi == &SPI_R) {
        osMutexAcquire(spi_mutex_r, osWaitForever);
    } else {
        Error_Handler();
    }
}

void api_hal_spi_unlock(SPI_HandleTypeDef* spi) {
    if(spi == &SPI_D) {
        osMutexRelease(spi_mutex_d);
    } else if(spi == &SPI_R) {
        osMutexRelease(spi_mutex_r);
    } else {
        Error_Handler();
    }
}

void api_hal_spi_lock_device(const SPIDevice* device) {
    api_hal_spi_lock(device->spi);
    api_hal_spi_config_device(device);
}

void api_hal_spi_unlock_device(const SPIDevice* device) {
    api_hal_spi_unlock(device->spi);
}