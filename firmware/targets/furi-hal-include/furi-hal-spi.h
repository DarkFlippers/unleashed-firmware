#pragma once
#include "main.h"
#include "furi-hal-spi-config.h"
#include <furi-hal-gpio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Init SPI API
 */
void furi_hal_spi_init();

/* Bus Level API */

/** Lock SPI bus
 * Takes bus mutex, if used
 */
void furi_hal_spi_bus_lock(const FuriHalSpiBus* bus);

/** Unlock SPI bus
 * Releases BUS mutex, if used
 */
void furi_hal_spi_bus_unlock(const FuriHalSpiBus* bus);

/** Configure SPI bus
 * @param bus - spi bus handler
 * @param config - spi configuration structure
 */
void furi_hal_spi_bus_configure(const FuriHalSpiBus* bus, const LL_SPI_InitTypeDef* config);

/** SPI Receive
 * @param bus - spi bus handler
 * @param buffer - receive buffer
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool furi_hal_spi_bus_rx(const FuriHalSpiBus* bus, uint8_t* buffer, size_t size, uint32_t timeout);

/** SPI Transmit
 * @param bus - spi bus handler
 * @param buffer - transmit buffer
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool furi_hal_spi_bus_tx(const FuriHalSpiBus* bus, uint8_t* buffer, size_t size, uint32_t timeout);

/** SPI Transmit and Receive
 * @param bus - spi bus handlere
 * @param tx_buffer - device handle
 * @param rx_buffer - device handle
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool furi_hal_spi_bus_trx(const FuriHalSpiBus* bus, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size, uint32_t timeout);

/* Device Level API */

/** Reconfigure SPI bus for device
 * @param device - device description
 */
void furi_hal_spi_device_configure(const FuriHalSpiDevice* device);

/** Get Device handle
 * And lock access to the corresponding SPI BUS
 * @param device_id - device identifier
 * @return device handle
 */
const FuriHalSpiDevice* furi_hal_spi_device_get(FuriHalSpiDeviceId device_id);

/** Return Device handle
 * And unlock access to the corresponding SPI BUS
 * @param device - device handle
 */
void furi_hal_spi_device_return(const FuriHalSpiDevice* device);

/** SPI Recieve
 * @param device - device handle
 * @param buffer - receive buffer
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool furi_hal_spi_device_rx(const FuriHalSpiDevice* device, uint8_t* buffer, size_t size, uint32_t timeout);

/** SPI Transmit
 * @param device - device handle
 * @param buffer - transmit buffer
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool furi_hal_spi_device_tx(const FuriHalSpiDevice* device, uint8_t* buffer, size_t size, uint32_t timeout);

/** SPI Transmit and Receive
 * @param device - device handle
 * @param tx_buffer - device handle
 * @param rx_buffer - device handle
 * @param size - transaction size
 * @param timeout - bus operation timeout in ms
 */
bool furi_hal_spi_device_trx(const FuriHalSpiDevice* device, uint8_t* tx_buffer, uint8_t* rx_buffer, size_t size, uint32_t timeout);


#ifdef __cplusplus
}
#endif