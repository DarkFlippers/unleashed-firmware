#pragma once
#include "main.h"
#include "api-hal-spi-config.h"
#include <api-hal-gpio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Init SPI API
 */
void api_hal_spi_init();

/* Bus Level API */

/** Lock SPI bus
 * Takes bus mutex, if used
 */
void api_hal_spi_bus_lock(const ApiHalSpiBus* bus);

/** Unlock SPI bus
 * Releases BUS mutex, if used
 */
void api_hal_spi_bus_unlock(const ApiHalSpiBus* bus);

/** SPI Receive
 * @param bus - spi bus handler
 * @param buffer - receive buffer
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool api_hal_spi_bus_rx(const ApiHalSpiBus* bus, uint8_t* buffer, size_t size, uint32_t timeout);

/** SPI Transmit
 * @param bus - spi bus handler
 * @param buffer - transmit buffer
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool api_hal_spi_bus_tx(const ApiHalSpiBus* bus, uint8_t* buffer, size_t size, uint32_t timeout);

/** SPI Transmit and Receive
 * @param bus - spi bus handlere
 * @param tx_buffer - device handle
 * @param rx_buffer - device handle
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool api_hal_spi_bus_trx(const ApiHalSpiBus* bus, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size, uint32_t timeout);

/* Device Level API */

/** Get Device handle
 * And lock access to the corresponding SPI BUS
 * @param device_id - device identifier
 * @return device handle
 */
const ApiHalSpiDevice* api_hal_spi_device_get(ApiHalSpiDeviceId device_id);

/** Return Device handle
 * And unlock access to the corresponding SPI BUS
 * @param device - device handle
 */
void api_hal_spi_device_return(const ApiHalSpiDevice* device);

/** SPI Recieve
 * @param device - device handle
 * @param buffer - receive buffer
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool api_hal_spi_device_rx(const ApiHalSpiDevice* device, uint8_t* buffer, size_t size, uint32_t timeout);

/** SPI Transmit
 * @param device - device handle
 * @param buffer - transmit buffer
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool api_hal_spi_device_tx(const ApiHalSpiDevice* device, uint8_t* buffer, size_t size, uint32_t timeout);

/** SPI Transmit and Receive
 * @param device - device handle
 * @param tx_buffer - device handle
 * @param rx_buffer - device handle
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool api_hal_spi_device_trx(const ApiHalSpiDevice* device, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size, uint32_t timeout);


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