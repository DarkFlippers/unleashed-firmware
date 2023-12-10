#pragma once

#include <furi_hal_i2c_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Internal(power) i2c bus, I2C1, under reset when not used */
extern FuriHalI2cBus furi_hal_i2c_bus_power;

/** External i2c bus, I2C3, under reset when not used */
extern FuriHalI2cBus furi_hal_i2c_bus_external;

/** Handle for internal(power) i2c bus
 * Bus: furi_hal_i2c_bus_external
 * Pins: PA9(SCL) / PA10(SDA), float on release
 * Params: 400khz
 */
extern FuriHalI2cBusHandle furi_hal_i2c_handle_power;

/** Handle for external i2c bus
 * Bus: furi_hal_i2c_bus_external
 * Pins: PC0(SCL) / PC1(SDA), float on release
 * Params: 100khz
 */
extern FuriHalI2cBusHandle furi_hal_i2c_handle_external;

#ifdef __cplusplus
}
#endif
