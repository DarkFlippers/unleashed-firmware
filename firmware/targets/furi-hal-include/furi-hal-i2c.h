/**
 * @file furi-hal-i2c.h
 * I2C HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi-hal-i2c-config.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Init I2C
 */
void furi_hal_i2c_init();

/** Acquire i2c bus handle
 *
 * @return     Instance of FuriHalI2cBus
 */
void furi_hal_i2c_acquire(FuriHalI2cBusHandle* handle);

/** Release i2c bus handle
 *
 * @param      bus   instance of FuriHalI2cBus aquired in `furi_hal_i2c_acquire`
 */
void furi_hal_i2c_release(FuriHalI2cBusHandle* handle);

/** Perform I2C tx transfer
 *
 * @param      handle   pointer to FuriHalI2cBusHandle instance
 * @param      address  I2C slave address
 * @param      data     pointer to data buffer
 * @param      size     size of data buffer
 * @param      timeout  timeout in ticks
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_tx(
    FuriHalI2cBusHandle* handle,
    const uint8_t address,
    const uint8_t* data,
    const uint8_t size,
    uint32_t timeout);

/** Perform I2C rx transfer
 *
 * @param      handle   pointer to FuriHalI2cBusHandle instance
 * @param      address  I2C slave address
 * @param      data     pointer to data buffer
 * @param      size     size of data buffer
 * @param      timeout  timeout in ticks
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_rx(
    FuriHalI2cBusHandle* handle,
    const uint8_t address,
    uint8_t* data,
    const uint8_t size,
    uint32_t timeout);

/** Perform I2C tx and rx transfers
 *
 * @param      handle   pointer to FuriHalI2cBusHandle instance
 * @param      address  I2C slave address
 * @param      tx_data  pointer to tx data buffer
 * @param      tx_size  size of tx data buffer
 * @param      rx_data  pointer to rx data buffer
 * @param      rx_size  size of rx data buffer
 * @param      timeout  timeout in ticks
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_trx(
    FuriHalI2cBusHandle* handle,
    const uint8_t address,
    const uint8_t* tx_data,
    const uint8_t tx_size,
    uint8_t* rx_data,
    const uint8_t rx_size,
    uint32_t timeout);

#ifdef __cplusplus
}
#endif
