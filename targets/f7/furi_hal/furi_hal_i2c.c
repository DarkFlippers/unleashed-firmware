#include <furi_hal_i2c.h>
#include <furi_hal_version.h>
#include <furi_hal_power.h>
#include <furi_hal_cortex.h>

#include <stm32wbxx_ll_i2c.h>
#include <stm32wbxx_ll_gpio.h>
#include <furi.h>

#define TAG "FuriHalI2c"

void furi_hal_i2c_init_early(void) {
    furi_hal_i2c_bus_power.callback(&furi_hal_i2c_bus_power, FuriHalI2cBusEventInit);
}

void furi_hal_i2c_deinit_early(void) {
    furi_hal_i2c_bus_power.callback(&furi_hal_i2c_bus_power, FuriHalI2cBusEventDeinit);
}

void furi_hal_i2c_init(void) {
    furi_hal_i2c_bus_external.callback(&furi_hal_i2c_bus_external, FuriHalI2cBusEventInit);
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_i2c_acquire(FuriHalI2cBusHandle* handle) {
    furi_hal_power_insomnia_enter();
    // Lock bus access
    handle->bus->callback(handle->bus, FuriHalI2cBusEventLock);
    // Ensure that no active handle set
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

static bool
    furi_hal_i2c_wait_for_idle(I2C_TypeDef* i2c, FuriHalI2cBegin begin, FuriHalCortexTimer timer) {
    do {
        if(furi_hal_cortex_timer_is_expired(timer)) {
            return false;
        }
    } while(begin == FuriHalI2cBeginStart && LL_I2C_IsActiveFlag_BUSY(i2c));
    // Only check if the bus is busy if starting a new transaction, if not we already control the bus

    return true;
}

static bool
    furi_hal_i2c_wait_for_end(I2C_TypeDef* i2c, FuriHalI2cEnd end, FuriHalCortexTimer timer) {
    // If ending the transaction with a stop condition, wait for it to be detected, otherwise wait for a transfer complete flag
    bool wait_for_stop = end == FuriHalI2cEndStop;
    uint32_t end_mask = (wait_for_stop) ? I2C_ISR_STOPF : (I2C_ISR_TC | I2C_ISR_TCR);

    while((i2c->ISR & end_mask) == 0) {
        if(furi_hal_cortex_timer_is_expired(timer)) {
            return false;
        }
    }

    return true;
}

static uint32_t
    furi_hal_i2c_get_start_signal(FuriHalI2cBegin begin, bool ten_bit_address, bool read) {
    switch(begin) {
    case FuriHalI2cBeginRestart:
        if(read) {
            return ten_bit_address ? LL_I2C_GENERATE_RESTART_10BIT_READ :
                                     LL_I2C_GENERATE_RESTART_7BIT_READ;
        } else {
            return ten_bit_address ? LL_I2C_GENERATE_RESTART_10BIT_WRITE :
                                     LL_I2C_GENERATE_RESTART_7BIT_WRITE;
        }
    case FuriHalI2cBeginResume:
        return LL_I2C_GENERATE_NOSTARTSTOP;
    case FuriHalI2cBeginStart:
    default:
        return read ? LL_I2C_GENERATE_START_READ : LL_I2C_GENERATE_START_WRITE;
    }
}

static uint32_t furi_hal_i2c_get_end_signal(FuriHalI2cEnd end) {
    switch(end) {
    case FuriHalI2cEndAwaitRestart:
        return LL_I2C_MODE_SOFTEND;
    case FuriHalI2cEndPause:
        return LL_I2C_MODE_RELOAD;
    case FuriHalI2cEndStop:
    default:
        return LL_I2C_MODE_AUTOEND;
    }
}

static bool furi_hal_i2c_transfer_is_aborted(I2C_TypeDef* i2c) {
    return LL_I2C_IsActiveFlag_STOP(i2c) &&
           !(LL_I2C_IsActiveFlag_TC(i2c) || LL_I2C_IsActiveFlag_TCR(i2c));
}

static bool furi_hal_i2c_transfer(
    I2C_TypeDef* i2c,
    uint8_t* data,
    uint32_t size,
    FuriHalI2cEnd end,
    bool read,
    FuriHalCortexTimer timer) {
    bool ret = true;

    while(size > 0) {
        bool should_stop = furi_hal_cortex_timer_is_expired(timer) ||
                           furi_hal_i2c_transfer_is_aborted(i2c);

        // Modifying the data pointer's data is UB if read is true
        if(read && LL_I2C_IsActiveFlag_RXNE(i2c)) {
            *data = LL_I2C_ReceiveData8(i2c);
            data++;
            size--;
        } else if(!read && LL_I2C_IsActiveFlag_TXIS(i2c)) {
            LL_I2C_TransmitData8(i2c, *data);
            data++;
            size--;
        }

        // Exit on timeout or premature stop, probably caused by a nacked address or byte
        if(should_stop) {
            ret = size == 0; // If the transfer was over, still a success
            break;
        }
    }

    if(ret) {
        ret = furi_hal_i2c_wait_for_end(i2c, end, timer);
    }

    LL_I2C_ClearFlag_STOP(i2c);

    return ret;
}

static bool furi_hal_i2c_transaction(
    I2C_TypeDef* i2c,
    uint16_t address,
    bool ten_bit,
    uint8_t* data,
    size_t size,
    FuriHalI2cBegin begin,
    FuriHalI2cEnd end,
    bool read,
    FuriHalCortexTimer timer) {
    uint32_t addr_size = ten_bit ? LL_I2C_ADDRSLAVE_10BIT : LL_I2C_ADDRSLAVE_7BIT;
    uint32_t start_signal = furi_hal_i2c_get_start_signal(begin, ten_bit, read);

    if(!furi_hal_i2c_wait_for_idle(i2c, begin, timer)) {
        return false;
    }

    do {
        uint8_t transfer_size = size;
        FuriHalI2cEnd transfer_end = end;

        if(size > 255) {
            transfer_size = 255;
            transfer_end = FuriHalI2cEndPause;
        }

        uint32_t end_signal = furi_hal_i2c_get_end_signal(transfer_end);

        LL_I2C_HandleTransfer(i2c, address, addr_size, transfer_size, end_signal, start_signal);

        if(!furi_hal_i2c_transfer(i2c, data, transfer_size, transfer_end, read, timer)) {
            return false;
        }

        size -= transfer_size;
        data += transfer_size;
        start_signal = LL_I2C_GENERATE_NOSTARTSTOP;
    } while(size > 0);

    return true;
}

bool furi_hal_i2c_rx_ext(
    FuriHalI2cBusHandle* handle,
    uint16_t address,
    bool ten_bit,
    uint8_t* data,
    size_t size,
    FuriHalI2cBegin begin,
    FuriHalI2cEnd end,
    uint32_t timeout) {
    furi_check(handle->bus->current_handle == handle);

    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout * 1000);

    return furi_hal_i2c_transaction(
        handle->bus->i2c, address, ten_bit, data, size, begin, end, true, timer);
}

bool furi_hal_i2c_tx_ext(
    FuriHalI2cBusHandle* handle,
    uint16_t address,
    bool ten_bit,
    const uint8_t* data,
    size_t size,
    FuriHalI2cBegin begin,
    FuriHalI2cEnd end,
    uint32_t timeout) {
    furi_check(handle->bus->current_handle == handle);

    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout * 1000);

    return furi_hal_i2c_transaction(
        handle->bus->i2c, address, ten_bit, (uint8_t*)data, size, begin, end, false, timer);
}

bool furi_hal_i2c_tx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    const uint8_t* data,
    size_t size,
    uint32_t timeout) {
    furi_check(timeout > 0);

    return furi_hal_i2c_tx_ext(
        handle, address, false, data, size, FuriHalI2cBeginStart, FuriHalI2cEndStop, timeout);
}

bool furi_hal_i2c_rx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    uint8_t* data,
    size_t size,
    uint32_t timeout) {
    furi_check(timeout > 0);

    return furi_hal_i2c_rx_ext(
        handle, address, false, data, size, FuriHalI2cBeginStart, FuriHalI2cEndStop, timeout);
}

bool furi_hal_i2c_trx(
    FuriHalI2cBusHandle* handle,
    uint8_t address,
    const uint8_t* tx_data,
    size_t tx_size,
    uint8_t* rx_data,
    size_t rx_size,
    uint32_t timeout) {
    return furi_hal_i2c_tx_ext(
               handle,
               address,
               false,
               tx_data,
               tx_size,
               FuriHalI2cBeginStart,
               FuriHalI2cEndStop,
               timeout) &&
           furi_hal_i2c_rx_ext(
               handle,
               address,
               false,
               rx_data,
               rx_size,
               FuriHalI2cBeginStart,
               FuriHalI2cEndStop,
               timeout);
}

bool furi_hal_i2c_is_device_ready(FuriHalI2cBusHandle* handle, uint8_t i2c_addr, uint32_t timeout) {
    furi_check(handle);
    furi_check(handle->bus->current_handle == handle);
    furi_check(timeout > 0);

    bool ret = true;
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(timeout * 1000);

    if(!furi_hal_i2c_wait_for_idle(handle->bus->i2c, FuriHalI2cBeginStart, timer)) {
        return false;
    }

    LL_I2C_HandleTransfer(
        handle->bus->i2c,
        i2c_addr,
        LL_I2C_ADDRSLAVE_7BIT,
        0,
        LL_I2C_MODE_AUTOEND,
        LL_I2C_GENERATE_START_WRITE);

    if(!furi_hal_i2c_wait_for_end(handle->bus->i2c, FuriHalI2cEndStop, timer)) {
        return false;
    }

    ret = !LL_I2C_IsActiveFlag_NACK(handle->bus->i2c);

    LL_I2C_ClearFlag_NACK(handle->bus->i2c);
    LL_I2C_ClearFlag_STOP(handle->bus->i2c);

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
    size_t len,
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

    const uint8_t tx_data[2] = {
        reg_addr,
        data,
    };

    return furi_hal_i2c_tx(handle, i2c_addr, tx_data, 2, timeout);
}

bool furi_hal_i2c_write_reg_16(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t reg_addr,
    uint16_t data,
    uint32_t timeout) {
    furi_check(handle);

    const uint8_t tx_data[3] = {
        reg_addr,
        (data >> 8) & 0xFF,
        data & 0xFF,
    };

    return furi_hal_i2c_tx(handle, i2c_addr, tx_data, 3, timeout);
}

bool furi_hal_i2c_write_mem(
    FuriHalI2cBusHandle* handle,
    uint8_t i2c_addr,
    uint8_t mem_addr,
    const uint8_t* data,
    size_t len,
    uint32_t timeout) {
    furi_check(handle);
    furi_check(handle->bus->current_handle == handle);
    furi_check(timeout > 0);

    return furi_hal_i2c_tx_ext(
               handle,
               i2c_addr,
               false,
               &mem_addr,
               1,
               FuriHalI2cBeginStart,
               FuriHalI2cEndPause,
               timeout) &&
           furi_hal_i2c_tx_ext(
               handle,
               i2c_addr,
               false,
               data,
               len,
               FuriHalI2cBeginResume,
               FuriHalI2cEndStop,
               timeout);
}
