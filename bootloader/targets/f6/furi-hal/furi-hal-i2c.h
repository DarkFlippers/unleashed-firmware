#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi-hal-resources.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_hal_i2c_init();

bool furi_hal_i2c_tx(
    I2C_TypeDef* instance,
    const uint8_t address,
    const uint8_t* data,
    const uint8_t size,
    uint32_t timeout);

bool furi_hal_i2c_rx(
    I2C_TypeDef* instance,
    const uint8_t address,
    uint8_t* data,
    const uint8_t size,
    uint32_t timeout);

bool furi_hal_i2c_trx(
    I2C_TypeDef* instance,
    const uint8_t address,
    const uint8_t* tx_data,
    const uint8_t tx_size,
    uint8_t* rx_data,
    const uint8_t rx_size,
    uint32_t timeout);

#define with_furi_hal_i2c(type, pointer, function_body)       \
    {                                                         \
        *pointer = ({ type __fn__ function_body __fn__; })(); \
    }

#ifdef __cplusplus
}
#endif
