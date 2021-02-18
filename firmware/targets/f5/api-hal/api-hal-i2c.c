#include <api-hal-i2c.h>
#include <furi.h>

osMutexId_t api_hal_i2c_mutex = NULL;

void api_hal_i2c_init() {
    api_hal_i2c_mutex = osMutexNew(NULL);
    furi_check(api_hal_i2c_mutex);
}

void api_hal_i2c_lock() {
    furi_check(osMutexAcquire(api_hal_i2c_mutex, osWaitForever) == osOK);
}

void api_hal_i2c_unlock() {
    furi_check(osMutexRelease(api_hal_i2c_mutex) == osOK);
}
