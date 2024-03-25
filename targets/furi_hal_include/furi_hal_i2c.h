/**
 * @file furi_hal_i2c.h I2C HAL API
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <furi_hal_i2c_config.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Transaction beginning signal */
typedef enum {
    /*!Begin the transaction by sending a START condition followed by the
     * address */
    FuriHalI2cBeginStart,
    /*!Begin the transaction by sending a RESTART condition followed by the
     * address
     * @note       Must follow a transaction ended with
     *             FuriHalI2cEndAwaitRestart */
    FuriHalI2cBeginRestart,
    /*!Continue the previous transaction with new data
     * @note       Must follow a transaction ended with FuriHalI2cEndPause and
     *             be of the same type (RX/TX) */
    FuriHalI2cBeginResume,
} FuriHalI2cBegin;

/** Transaction end signal */
typedef enum {
    /*!End the transaction by sending a STOP condition */
    FuriHalI2cEndStop,
    /*!End the transaction by clock stretching
     * @note       Must be followed by a transaction using
     *             FuriHalI2cBeginRestart */
    FuriHalI2cEndAwaitRestart,
    /*!Pauses the transaction by clock stretching
     * @note       Must be followed by a transaction using FuriHalI2cBeginResume */
    FuriHalI2cEndPause,
} FuriHalI2cEnd;

/** Early Init I2C */
void furi_hal_i2c_init_early(void);

/** Early DeInit I2C */
void furi_hal_i2c_deinit_early(void);

/** Init I2C */
void furi_hal_i2c_init(void);

/** Acquire I2C bus handle
 *
 * @param      handle  Pointer to FuriHalI2cBusHandle instance
 */
void furi_hal_i2c_acquire(FuriHalI2cBusHandle* handle);

/** Release I2C bus handle
 * 
 * @param      handle  Pointer to FuriHalI2cBusHandle instance acquired in
 *                     `furi_hal_i2c_acquire`
 */
void furi_hal_i2c_release(FuriHalI2cBusHandle* handle);

/** Perform I2C TX transfer
 *
 * @param      handle   Pointer to FuriHalI2cBusHandle instance
 * @param      address  I2C slave address
 * @param      data     Pointer to data buffer
 * @param      size     Size of data buffer
 * @param      timeout  Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_tx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    const uint8_t* data,
    size_t size,
    uint32_t timeout);

/**
 * Perform I2C TX transfer, with additional settings.
 *
 * @param      handle   Pointer to FuriHalI2cBusHandle instance
 * @param      address  I2C slave address
 * @param      ten_bit  Whether the address is 10 bits wide
 * @param      data     Pointer to data buffer
 * @param      size     Size of data buffer
 * @param      begin    How to begin the transaction
 * @param      end      How to end the transaction
 * @param      timeout  Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_tx_ext(
    FuriHalI2cBusHandle* handle,
    uint16_t address,
    bool ten_bit,
    const uint8_t* data,
    size_t size,
    FuriHalI2cBegin begin,
    FuriHalI2cEnd end,
    uint32_t timeout);

/** Perform I2C RX transfer
 *
 * @param      handle   Pointer to FuriHalI2cBusHandle instance
 * @param      address  I2C slave address
 * @param      data     Pointer to data buffer
 * @param      size     Size of data buffer
 * @param      timeout  Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_rx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    uint8_t* data,
    size_t size,
    uint32_t timeout);

/** Perform I2C RX transfer, with additional settings.
 *
 * @param      handle   Pointer to FuriHalI2cBusHandle instance
 * @param      address  I2C slave address
 * @param      ten_bit  Whether the address is 10 bits wide
 * @param      data     Pointer to data buffer
 * @param      size     Size of data buffer
 * @param      begin    How to begin the transaction
 * @param      end      How to end the transaction
 * @param      timeout  Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_rx_ext(
    FuriHalI2cBusHandle* handle,
    uint16_t address,
    bool ten_bit,
    uint8_t* data,
    size_t size,
    FuriHalI2cBegin begin,
    FuriHalI2cEnd end,
    uint32_t timeout);

/** Perform I2C TX and RX transfers
 *
 * @param      handle   Pointer to FuriHalI2cBusHandle instance
 * @param      address  I2C slave address
 * @param      tx_data  Pointer to TX data buffer
 * @param      tx_size  Size of TX data buffer
 * @param      rx_data  Pointer to RX data buffer
 * @param      rx_size  Size of RX data buffer
 * @param      timeout  Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_trx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    const uint8_t* tx_data,
    size_t tx_size,
    uint8_t* rx_data,
    size_t rx_size,
    uint32_t timeout);

/** Check if I2C device presents on bus
 *
 * @param      handle    Pointer to FuriHalI2cBusHandle instance
 * @param      i2c_addr  I2C slave address
 * @param      timeout   Timeout in milliseconds
 *
 * @return     true if device present and is ready, false otherwise
 */
bool furi_hal_i2c_is_device_ready(FuriHalI2cBusHandle* handle, uint8_t i2c_addr, uint32_t timeout);

/** Perform I2C device register read (8-bit)
 *
 * @param      handle    Pointer to FuriHalI2cBusHandle instance
 * @param      i2c_addr  I2C slave address
 * @param      reg_addr  Register address
 * @param      data      Pointer to register value
 * @param      timeout   Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_read_reg_8(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint8_t* data,
    uint32_t timeout);

/** Perform I2C device register read (16-bit)
 *
 * @param      handle    Pointer to FuriHalI2cBusHandle instance
 * @param      i2c_addr  I2C slave address
 * @param      reg_addr  Register address
 * @param      data      Pointer to register value
 * @param      timeout   Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_read_reg_16(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint16_t* data,
    uint32_t timeout);

/** Perform I2C device memory read
 *
 * @param      handle    Pointer to FuriHalI2cBusHandle instance
 * @param      i2c_addr  I2C slave address
 * @param      mem_addr  Memory start address
 * @param      data      Pointer to data buffer
 * @param      len       Size of data buffer
 * @param      timeout   Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_read_mem(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t mem_addr,
    uint8_t* data,
    size_t len,
    uint32_t timeout);

/** Perform I2C device register write (8-bit)
 *
 * @param      handle    Pointer to FuriHalI2cBusHandle instance
 * @param      i2c_addr  I2C slave address
 * @param      reg_addr  Register address
 * @param      data      Register value
 * @param      timeout   Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_write_reg_8(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint8_t data,
    uint32_t timeout);

/** Perform I2C device register write (16-bit)
 *
 * @param      handle    Pointer to FuriHalI2cBusHandle instance
 * @param      i2c_addr  I2C slave address
 * @param      reg_addr  Register address
 * @param      data      Register value
 * @param      timeout   Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_write_reg_16(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint16_t data,
    uint32_t timeout);

/** Perform I2C device memory
 *
 * @param      handle    Pointer to FuriHalI2cBusHandle instance
 * @param      i2c_addr  I2C slave address
 * @param      mem_addr  Memory start address
 * @param      data      Pointer to data buffer
 * @param      len       Size of data buffer
 * @param      timeout   Timeout in milliseconds
 *
 * @return     true on successful transfer, false otherwise
 */
bool furi_hal_i2c_write_mem(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t mem_addr,
    const uint8_t* data,
    size_t len,
    uint32_t timeout);

#ifdef __cplusplus
}
#endif
