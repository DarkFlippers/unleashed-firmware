/**
 * @file furi-hal-i2c.h
 * I2C HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi-hal-resources.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Init I2C
 */
void furi_hal_i2c_init();

/** Perform I2C tx transfer
 *
 * @param      instance  I2C_TypeDef instance
 * @param      address   I2C slave address
 * @param      data      pointer to data buffer
 * @param      size      size of data buffer
 * @param      timeout   timeout in CPU ticks
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_tx(
    I2C_TypeDef* instance,
    const uint8_t address,
    const uint8_t* data,
    const uint8_t size,
    uint32_t timeout);

/** Perform I2C rx transfer
 *
 * @param      instance  I2C_TypeDef instance
 * @param      address   I2C slave address
 * @param      data      pointer to data buffer
 * @param      size      size of data buffer
 * @param      timeout   timeout in CPU ticks
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_rx(
    I2C_TypeDef* instance,
    const uint8_t address,
    uint8_t* data,
    const uint8_t size,
    uint32_t timeout);

/** Perform I2C tx and rx transfers
 *
 * @param      instance  I2C_TypeDef instance
 * @param      address   I2C slave address
 * @param      tx_data   pointer to tx data buffer
 * @param      tx_size   size of tx data buffer
 * @param      rx_data   pointer to rx data buffer
 * @param      rx_size   size of rx data buffer
 * @param      timeout   timeout in CPU ticks
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_trx(
    I2C_TypeDef* instance,
    const uint8_t address,
    const uint8_t* tx_data,
    const uint8_t tx_size,
    uint8_t* rx_data,
    const uint8_t rx_size,
    uint32_t timeout);

/** Acquire I2C mutex
 */
void furi_hal_i2c_lock();

/** Release I2C mutex
 */
void furi_hal_i2c_unlock();

/** With clause for I2C peripheral
 *
 * @param      type           type of function_body
 * @param      pointer        pointer to return of function_body
 * @param      function_body  a (){} lambda declaration, executed with I2C mutex
 *                            acquired
 *
 * @return     Nothing
 */
#define with_furi_hal_i2c(type, pointer, function_body)        \
    {                                                         \
        furi_hal_i2c_lock();                                   \
        *pointer = ({ type __fn__ function_body __fn__; })(); \
        furi_hal_i2c_unlock();                                 \
    }

#ifdef __cplusplus
}
#endif
