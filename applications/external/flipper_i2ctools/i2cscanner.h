#pragma once

#include <furi.h>
#include <furi_hal.h>

// I2C BUS
#define I2C_BUS &furi_hal_i2c_handle_external
#define I2C_TIMEOUT 3

// 7 bits addresses
#define MAX_I2C_ADDR 0x7F

typedef struct {
    uint8_t addresses[MAX_I2C_ADDR + 1];
    uint8_t nb_found;
    uint8_t menu_index;
    bool scanned;
} i2cScanner;

void scan_i2c_bus(i2cScanner* i2c_scanner);

i2cScanner* i2c_scanner_alloc();
void i2c_scanner_free(i2cScanner* i2c_scanner);
