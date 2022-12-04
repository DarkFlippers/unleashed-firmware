#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <furi_hal_i2c.h>

#define FIRST_NON_RESERVED_I2C_ADDRESS 8
#define HIGHEST_I2C_ADDRESS 127
#define AVAILABLE_NONRESVERED_I2C_ADDRESSES 120

typedef struct {
    uint8_t items;
    uint8_t responding_address[AVAILABLE_NONRESVERED_I2C_ADDRESSES];
} I2CScannerState;

/** Scans the I2C-Bus (SDA: Pin 15, SCL: Pin 16) for available 7-Bit slave addresses. Saves the number of detected slaves and their addresses.
 *
 * @param      i2c_scanner_state  State including the detected addresses and the number of addresses saved.
 */
void gpio_i2c_scanner_run_once(I2CScannerState* st);
