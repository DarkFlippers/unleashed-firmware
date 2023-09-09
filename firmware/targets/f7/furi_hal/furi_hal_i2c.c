#include <furi_hal_i2c.h>
#include <furi_hal_version.h>
#include <furi_hal_power.h>
#include <furi_hal_cortex.h>

#include <stm32wbxx_ll_i2c.h>
#include <stm32wbxx_ll_gpio.h>
#include <stm32wbxx_ll_cortex.h>
#include <furi.h>

#define TAG "FuriHalI2c"

void furi_hal_i2c_init_early() {
    furi_hal_i2c_bus_power.callback(&furi_hal_i2c_bus_power, FuriHalI2cBusEventInit);
}

void furi_hal_i2c_deinit_early() {
    furi_hal_i2c_bus_power.callback(&furi_hal_i2c_bus_power, FuriHalI2cBusEventDeinit);
}

void furi_hal_i2c_init() {
    furi_hal_i2c_bus_external.callback(&furi_hal_i2c_bus_external, FuriHalI2cBusEventInit);
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_i2c_acquire(FuriHalI2cBusHandle* handle) {
    furi_hal_power_insomnia_enter();
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
    furi_hal_power_insomnia_exit();
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
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout * 1000);

    do {
        while(LL_I2C_IsActiveFlag_BUSY(handle->bus->i2c)) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
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

            if(furi_hal_cortex_timer_is_expired(timer)) {
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
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout * 1000);

    do {
        while(LL_I2C_IsActiveFlag_BUSY(handle->bus->i2c)) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
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

            if(furi_hal_cortex_timer_is_expired(timer)) {
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

bool furi_hal_i2c_is_device_ready(FuriHalI2cBusHandle* handle, uint8_t i2c_addr, uint32_t timeout) {
    furi_check(handle);

    furi_check(handle->bus->current_handle == handle);
    furi_assert(timeout > 0);

    bool ret = true;
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout * 1000);

    do {
        while(LL_I2C_IsActiveFlag_BUSY(handle->bus->i2c)) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                return false;
            }
        }

        handle->bus->i2c->CR2 =
            ((((uint32_t)(i2c_addr) & (I2C_CR2_SADD)) | (I2C_CR2_START) | (I2C_CR2_AUTOEND)) &
             (~I2C_CR2_RD_WRN));

        while((!LL_I2C_IsActiveFlag_NACK(handle->bus->i2c)) &&
              (!LL_I2C_IsActiveFlag_STOP(handle->bus->i2c))) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                return false;
            }
        }

        if(LL_I2C_IsActiveFlag_NACK(handle->bus->i2c)) {
            while(!LL_I2C_IsActiveFlag_STOP(handle->bus->i2c)) {
                if(furi_hal_cortex_timer_is_expired(timer)) {
                    return false;
                }
            }

            LL_I2C_ClearFlag_NACK(handle->bus->i2c);

            // Clear STOP Flag generated by autoend
            LL_I2C_ClearFlag_STOP(handle->bus->i2c);

            // Generate actual STOP
            LL_I2C_GenerateStopCondition(handle->bus->i2c);

            ret = false;
        }

        while(!LL_I2C_IsActiveFlag_STOP(handle->bus->i2c)) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                return false;
            }
        }

        LL_I2C_ClearFlag_STOP(handle->bus->i2c);
    } while(0);

    return ret;
}

bool furi_hal_i2c_read_reg_8(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint8_t* data,
    uint32_t timeout) {
    furi_check(handle);

    return furi_hal_i2c_trx(handle, i2c_addr, &reg_addr, 1, data, 1, timeout);
}

bool furi_hal_i2c_read_reg_16(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint16_t* data,
    uint32_t timeout) {
    furi_check(handle);

    uint8_t reg_data[2];
    bool ret = furi_hal_i2c_trx(handle, i2c_addr, &reg_addr, 1, reg_data, 2, timeout);
    *data = (reg_data[0] << 8) | (reg_data[1]);

    return ret;
}

bool furi_hal_i2c_read_mem(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t mem_addr,
    uint8_t* data,
    uint8_t len,
    uint32_t timeout) {
    furi_check(handle);

    return furi_hal_i2c_trx(handle, i2c_addr, &mem_addr, 1, data, len, timeout);
}

bool furi_hal_i2c_write_reg_8(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint8_t data,
    uint32_t timeout) {
    furi_check(handle);

    uint8_t tx_data[2];
    tx_data[0] = reg_addr;
    tx_data[1] = data;

    return furi_hal_i2c_tx(handle, i2c_addr, (const uint8_t*)&tx_data, 2, timeout);
}

bool furi_hal_i2c_write_reg_16(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint16_t data,
    uint32_t timeout) {
    furi_check(handle);

    uint8_t tx_data[3];
    tx_data[0] = reg_addr;
    tx_data[1] = (data >> 8) & 0xFF;
    tx_data[2] = data & 0xFF;

    return furi_hal_i2c_tx(handle, i2c_addr, (const uint8_t*)&tx_data, 3, timeout);
}

bool furi_hal_i2c_write_mem(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t mem_addr,
    uint8_t* data,
    uint8_t len,
    uint32_t timeout) {
    furi_check(handle);

    furi_check(handle->bus->current_handle == handle);
    furi_assert(timeout > 0);

    bool ret = true;
    uint8_t size = len + 1;
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout * 1000);

    do {
        while(LL_I2C_IsActiveFlag_BUSY(handle->bus->i2c)) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                ret = false;
                break;
            }
        }

        if(!ret) {
            break;
        }

        LL_I2C_HandleTransfer(
            handle->bus->i2c,
            i2c_addr,
            LL_I2C_ADDRSLAVE_7BIT,
            size,
            LL_I2C_MODE_AUTOEND,
            LL_I2C_GENERATE_START_WRITE);

        while(!LL_I2C_IsActiveFlag_STOP(handle->bus->i2c) || size > 0) {
            if(LL_I2C_IsActiveFlag_TXIS(handle->bus->i2c)) {
                if(size == len + 1) {
                    LL_I2C_TransmitData8(handle->bus->i2c, mem_addr);
                } else {
                    LL_I2C_TransmitData8(handle->bus->i2c, (*data));
                    data++;
                }
                size--;
            }

            if(furi_hal_cortex_timer_is_expired(timer)) {
                ret = false;
                break;
            }
        }

        LL_I2C_ClearFlag_STOP(handle->bus->i2c);
    } while(0);

    return ret;
}
