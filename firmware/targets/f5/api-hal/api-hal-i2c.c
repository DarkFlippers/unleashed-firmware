#include <api-hal-i2c.h>
#include <stm32wbxx_ll_i2c.h>
#include <furi.h>

osMutexId_t api_hal_i2c_mutex = NULL;

void api_hal_i2c_init() {
    api_hal_i2c_mutex = osMutexNew(NULL);
    furi_check(api_hal_i2c_mutex);
}

void api_hal_i2c_tx(I2C_TypeDef* instance, uint8_t address, const uint8_t *data, uint8_t size) {
    LL_I2C_HandleTransfer(instance, address, LL_I2C_ADDRSLAVE_7BIT, size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

    while (!LL_I2C_IsActiveFlag_STOP(instance)) {
        if (LL_I2C_IsActiveFlag_TXIS(instance)) {
            LL_I2C_TransmitData8(instance, (*data++));
        }
    }

    LL_I2C_ClearFlag_STOP(instance);
}

void api_hal_i2c_rx(I2C_TypeDef* instance, uint8_t address, uint8_t *data, uint8_t size) {
    LL_I2C_HandleTransfer(instance, address, LL_I2C_ADDRSLAVE_7BIT, size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

    while (!LL_I2C_IsActiveFlag_STOP(instance)) {
        if (LL_I2C_IsActiveFlag_RXNE(instance)) {
            *data++ = LL_I2C_ReceiveData8(instance);
        }
    }

    LL_I2C_ClearFlag_STOP(instance);
}

void api_hal_i2c_trx(I2C_TypeDef* instance, uint8_t address, const uint8_t *tx_data, uint8_t tx_size, uint8_t *rx_data, uint8_t rx_size) {
    api_hal_i2c_tx(instance, address, tx_data, tx_size);
    api_hal_i2c_rx(instance, address, rx_data, rx_size);
}

void api_hal_i2c_lock() {
    furi_check(osMutexAcquire(api_hal_i2c_mutex, osWaitForever) == osOK);
}

void api_hal_i2c_unlock() {
    furi_check(osMutexRelease(api_hal_i2c_mutex) == osOK);
}
