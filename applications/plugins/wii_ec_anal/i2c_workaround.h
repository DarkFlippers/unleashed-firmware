/*
	As of the date of releasing this code, there is (seemingly) a bug in the FZ i2c library code
	It is described here: https://github.com/flipperdevices/flipperzero-firmware/issues/1670

	This is a short-term workaround so I can keep developing while we get to the bottom of the issue

	FYI. *something* in the following code is the fix

void  furi_hal_i2c_acquire (FuriHalI2cBusHandle* handle)
{
	// 1. Disable the power/backlight (it uses i2c)
    furi_hal_power_insomnia_enter();
    // 2. Lock bus access
    handle->bus->callback(handle->bus, FuriHalI2cBusEventLock);
    // 3. Ensuree that no active handle set
    furi_check(handle->bus->current_handle == NULL);
    // 4. Set current handle
    handle->bus->current_handle = handle;
    // 5. Activate bus
    handle->bus->callback(handle->bus, FuriHalI2cBusEventActivate);
    // 6. Activate handle
    handle->callback(handle, FuriHalI2cBusHandleEventActivate);
}

void  furi_hal_i2c_release (FuriHalI2cBusHandle* handle)
{
    // Ensure that current handle is our handle
    furi_check(handle->bus->current_handle == handle);
    // 6. Deactivate handle
    handle->callback(handle, FuriHalI2cBusHandleEventDeactivate);
    // 5. Deactivate bus
    handle->bus->callback(handle->bus, FuriHalI2cBusEventDeactivate);
    // 3,4. Reset current handle
    handle->bus->current_handle = NULL;
    // 2. Unlock bus
    handle->bus->callback(handle->bus, FuriHalI2cBusEventUnlock);
	// 1. Re-enable the power system
    furi_hal_power_insomnia_exit();
}

*/

#ifndef I2C_WORKAROUND_H_
#define I2C_WORKAROUND_H_

#include <furi_hal.h>

#define ENABLE_WORKAROUND 1

#if ENABLE_WORKAROUND == 1
//+============================================================================ ========================================
static inline bool furi_hal_Wi2c_is_device_ready(
    FuriHalI2cBusHandle* const bus,
    const uint8_t addr,
    const uint32_t tmo) {
    furi_hal_i2c_acquire(bus);
    bool rv = furi_hal_i2c_is_device_ready(bus, addr, tmo);
    furi_hal_i2c_release(bus);
    return rv;
}

//+============================================================================
static inline bool furi_hal_Wi2c_tx(
    FuriHalI2cBusHandle* const bus,
    const uint8_t addr,
    const void* buf,
    const size_t len,
    const uint32_t tmo) {
    furi_hal_i2c_acquire(bus);
    bool rv = furi_hal_i2c_tx(bus, addr, buf, len, tmo);
    furi_hal_i2c_release(bus);
    return rv;
}

//+============================================================================
static inline bool furi_hal_Wi2c_rx(
    FuriHalI2cBusHandle* const bus,
    const uint8_t addr,
    void* buf,
    const size_t len,
    const uint32_t tmo) {
    furi_hal_i2c_acquire(bus);
    bool rv = furi_hal_i2c_rx(bus, addr, buf, len, tmo);
    furi_hal_i2c_release(bus);
    return rv;
}

//+============================================================================
static inline bool furi_hal_Wi2c_trx(
    FuriHalI2cBusHandle* const bus,
    const uint8_t addr,
    const void* tx,
    const size_t txlen,
    void* rx,
    const size_t rxlen,
    const uint32_t tmo) {
    bool rv = furi_hal_Wi2c_tx(bus, addr, tx, txlen, tmo);
    if(rv) rv = furi_hal_Wi2c_rx(bus, addr, rx, rxlen, tmo);
    return rv;
}

//----------------------------------------------------------------------------- ----------------------------------------
#define furi_hal_i2c_is_device_ready(...) furi_hal_Wi2c_is_device_ready(__VA_ARGS__)
#define furi_hal_i2c_tx(...) furi_hal_Wi2c_tx(__VA_ARGS__)
#define furi_hal_i2c_rx(...) furi_hal_Wi2c_rx(__VA_ARGS__)
#define furi_hal_i2c_trx(...) furi_hal_Wi2c_trx(__VA_ARGS__)

#endif //ENABLE_WORKAROUND

//+============================================================================ ========================================
// Some devices take a moment to respond to read requests
// The puts a delay between the address being set and the data being read
//
static inline bool furi_hal_i2c_trxd(
    FuriHalI2cBusHandle* const bus,
    const uint8_t addr,
    const void* tx,
    const size_t txlen,
    void* rx,
    const size_t rxlen,
    const uint32_t tmo,
    const uint32_t us) {
    bool rv = furi_hal_i2c_tx(bus, addr, tx, txlen, tmo);
    if(rv) {
        furi_delay_us(us);
        rv = furi_hal_i2c_rx(bus, addr, rx, rxlen, tmo);
    }
    return rv;
}

#endif //I2C_WORKAROUND_H_
