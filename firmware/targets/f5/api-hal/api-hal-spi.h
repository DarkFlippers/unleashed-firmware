#pragma once
#include "main.h"
#include "api-hal-spi-config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Init SPI API
 */
void api_hal_spi_init();

/**
 * Lock SPI bus
 */
void api_hal_spi_lock(SPI_HandleTypeDef* spi);

/**
 * Unlock SPI bus
 */
void api_hal_spi_unlock(SPI_HandleTypeDef* spi);

/**
 * Lock SPI device bus and apply config if needed
 */
void api_hal_spi_lock_device(const SPIDevice* device);

/**
 * Unlock SPI device bus
 */
void api_hal_spi_unlock_device(const SPIDevice* device);

#ifdef __cplusplus
}
#endif