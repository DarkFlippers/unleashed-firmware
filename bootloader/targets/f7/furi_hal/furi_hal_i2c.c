#include <furi_hal_i2c.h>
#include <furi_hal_version.h>

#include <stm32wbxx_ll_i2c.h>
#include <stm32wbxx_ll_gpio.h>
#include <stm32wbxx_ll_cortex.h>

#include <assert.h>

void furi_hal_i2c_init() {
    furi_hal_i2c_bus_power.callback(&furi_hal_i2c_bus_power, FuriHalI2cBusEventInit);
}

void furi_hal_i2c_acquire(FuriHalI2cBusHandle* handle) {
    handle->bus->callback(handle->bus, FuriHalI2cBusEventLock);

    assert(handle->bus->current_handle == NULL);

    handle->bus->current_handle = handle;

    handle->bus->callback(handle->bus, FuriHalI2cBusEventActivate);

    handle->callback(handle, FuriHalI2cBusHandleEventActivate);
}

void furi_hal_i2c_release(FuriHalI2cBusHandle* handle) {
    assert(handle->bus->current_handle == handle);

    handle->callback(handle, FuriHalI2cBusHandleEventDeactivate);

    handle->bus->callback(handle->bus, FuriHalI2cBusEventDeactivate);

    handle->bus->current_handle = NULL;

    handle->bus->callback(handle->bus, FuriHalI2cBusEventUnlock);
}

bool furi_hal_i2c_tx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    const uint8_t* data,
    uint8_t size,
    uint32_t timeout) {
    assert(handle->bus->current_handle == handle);
    uint32_t time_left = timeout;
    bool ret = true;

    while(LL_I2C_IsActiveFlag_BUSY(handle->bus->i2c))
        ;

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
            time_left = timeout;
        }

        if(LL_SYSTICK_IsActiveCounterFlag()) {
            if(--time_left == 0) {
                ret = false;
                break;
            }
        }
    }

    LL_I2C_ClearFlag_STOP(handle->bus->i2c);

    return ret;
}

bool furi_hal_i2c_rx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    uint8_t* data,
    uint8_t size,
    uint32_t timeout) {
    assert(handle->bus->current_handle == handle);
    uint32_t time_left = timeout;
    bool ret = true;

    while(LL_I2C_IsActiveFlag_BUSY(handle->bus->i2c))
        ;

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
            time_left = timeout;
        }

        if(LL_SYSTICK_IsActiveCounterFlag()) {
            if(--time_left == 0) {
                ret = false;
                break;
            }
        }
    }

    LL_I2C_ClearFlag_STOP(handle->bus->i2c);

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

bool furi_hal_i2c_read_reg_8(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint8_t* data,
    uint32_t timeout) {
    assert(handle);

    return furi_hal_i2c_trx(handle, i2c_addr, &reg_addr, 1, data, 1, timeout);
}

bool furi_hal_i2c_read_reg_16(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint16_t* data,
    uint32_t timeout) {
    assert(handle);

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
    assert(handle);

    return furi_hal_i2c_trx(handle, i2c_addr, &mem_addr, 1, data, len, timeout);
}

bool furi_hal_i2c_write_reg_8(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint8_t data,
    uint32_t timeout) {
    assert(handle);

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
    assert(handle);

    uint8_t tx_data[3];
    tx_data[0] = reg_addr;
    tx_data[1] = (data >> 8) & 0xFF;
    tx_data[2] = data & 0xFF;

    return furi_hal_i2c_tx(handle, i2c_addr, (const uint8_t*)&tx_data, 3, timeout);
}
