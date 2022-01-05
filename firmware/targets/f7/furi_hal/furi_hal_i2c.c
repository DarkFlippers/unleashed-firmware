#include <furi_hal_i2c.h>
#include <furi_hal_version.h>

#include <stm32wbxx_ll_i2c.h>
#include <stm32wbxx_ll_gpio.h>
#include <stm32wbxx_ll_cortex.h>
#include <furi.h>

#define TAG "FuriHalI2C"

void furi_hal_i2c_init() {
    furi_hal_i2c_bus_power.callback(&furi_hal_i2c_bus_power, FuriHalI2cBusEventInit);
    furi_hal_i2c_bus_external.callback(&furi_hal_i2c_bus_external, FuriHalI2cBusEventInit);
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_i2c_acquire(FuriHalI2cBusHandle* handle) {
    // Lock bus access
    handle->bus->callback(handle->bus, FuriHalI2cBusEventLock);
    // Ensuree that no active handle set
    furi_check(handle->bus->current_handle == NULL);
    // Set current handle
    handle->bus->current_handle = handle;
    // Activate bus
    handle->bus->callback(handle->bus, FuriHalI2cBusEventActivate);
    // Activate handle
    handle->callback(handle, FuriHalI2cBusHandleEventActivate);
}

void furi_hal_i2c_release(FuriHalI2cBusHandle* handle) {
    // Ensure that current handle is our handle
    furi_check(handle->bus->current_handle == handle);
    // Deactivate handle
    handle->callback(handle, FuriHalI2cBusHandleEventDeactivate);
    // Deactivate bus
    handle->bus->callback(handle->bus, FuriHalI2cBusEventDeactivate);
    // Reset current handle
    handle->bus->current_handle = NULL;
    // Unlock bus
    handle->bus->callback(handle->bus, FuriHalI2cBusEventUnlock);
}

bool furi_hal_i2c_tx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    const uint8_t* data,
    uint8_t size,
    uint32_t timeout) {
    furi_check(handle->bus->current_handle == handle);
    furi_assert(timeout > 0);

    bool ret = true;
    uint32_t timeout_tick = HAL_GetTick() + timeout;

    do {
        while(LL_I2C_IsActiveFlag_BUSY(handle->bus->i2c)) {
            if(HAL_GetTick() >= timeout_tick) {
                ret = false;
                break;
            }
        }

        if(!ret) {
            break;
        }

        LL_I2C_HandleTransfer(
            handle->bus->i2c,
            address,
            LL_I2C_ADDRSLAVE_7BIT,
            size,
            LL_I2C_MODE_AUTOEND,
            LL_I2C_GENERATE_START_WRITE);

        while(!LL_I2C_IsActiveFlag_STOP(handle->bus->i2c) || size > 0) {
            if(LL_I2C_IsActiveFlag_TXIS(handle->bus->i2c)) {
                LL_I2C_TransmitData8(handle->bus->i2c, (*data));
                data++;
                size--;
            }

            if(HAL_GetTick() >= timeout_tick) {
                ret = false;
                break;
            }
        }

        LL_I2C_ClearFlag_STOP(handle->bus->i2c);
    } while(0);

    return ret;
}

bool furi_hal_i2c_rx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    uint8_t* data,
    uint8_t size,
    uint32_t timeout) {
    furi_check(handle->bus->current_handle == handle);
    furi_assert(timeout > 0);

    bool ret = true;
    uint32_t timeout_tick = HAL_GetTick() + timeout;

    do {
        while(LL_I2C_IsActiveFlag_BUSY(handle->bus->i2c)) {
            if(HAL_GetTick() >= timeout_tick) {
                ret = false;
                break;
            }
        }

        if(!ret) {
            break;
        }

        LL_I2C_HandleTransfer(
            handle->bus->i2c,
            address,
            LL_I2C_ADDRSLAVE_7BIT,
            size,
            LL_I2C_MODE_AUTOEND,
            LL_I2C_GENERATE_START_READ);

        while(!LL_I2C_IsActiveFlag_STOP(handle->bus->i2c) || size > 0) {
            if(LL_I2C_IsActiveFlag_RXNE(handle->bus->i2c)) {
                *data = LL_I2C_ReceiveData8(handle->bus->i2c);
                data++;
                size--;
            }

            if(HAL_GetTick() >= timeout_tick) {
                ret = false;
                break;
            }
        }

        LL_I2C_ClearFlag_STOP(handle->bus->i2c);
    } while(0);

    return ret;
}

bool furi_hal_i2c_trx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    const uint8_t* tx_data,
    uint8_t tx_size,
    uint8_t* rx_data,
    uint8_t rx_size,
    uint32_t timeout) {
    if(furi_hal_i2c_tx(handle, address, tx_data, tx_size, timeout) &&
       furi_hal_i2c_rx(handle, address, rx_data, rx_size, timeout)) {
        return true;
    } else {
        return false;
    }
}
