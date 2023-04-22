/**
 * @file furi_hal_power.h
 * Power HAL API
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <core/string.h>
#include <toolbox/property.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Power IC type */
typedef enum {
    FuriHalPowerICCharger,
    FuriHalPowerICFuelGauge,
} FuriHalPowerIC;

/** Initialize drivers */
void furi_hal_power_init();

/** Check if gauge is ok
 * 
 * Verifies that:
 * - gauge is alive
 * - correct profile loaded
 * - self diagnostic status is good
 * 
 * @return true if gauge is ok
 */
bool furi_hal_power_gauge_is_ok();

/** Get current insomnia level
 *
 * @return     insomnia level: 0 - no insomnia, >0 - insomnia, bearer count.
 */
uint16_t furi_hal_power_insomnia_level();

/** Enter insomnia mode Prevents device from going to sleep
 * @warning    Internally increases insomnia level Must be paired with
 *             furi_hal_power_insomnia_exit
 */
void furi_hal_power_insomnia_enter();

/** Exit insomnia mode Allow device to go to sleep
 * @warning    Internally decreases insomnia level. Must be paired with
 *             furi_hal_power_insomnia_enter
 */
void furi_hal_power_insomnia_exit();

/** Check if sleep availble
 *
 * @return     true if available
 */
bool furi_hal_power_sleep_available();

/** Check if deep sleep availble
 *
 * @return     true if available
 */
bool furi_hal_power_deep_sleep_available();

/** Go to sleep
 */
void furi_hal_power_sleep();

/** Get predicted remaining battery capacity in percents
 *
 * @return     remaining battery capacity in percents
 */
uint8_t furi_hal_power_get_pct();

/** Get battery health state in percents
 *
 * @return     health in percents
 */
uint8_t furi_hal_power_get_bat_health_pct();

/** Get charging status
 *
 * @return     true if charging
 */
bool furi_hal_power_is_charging();

/** Get charge complete status
 *
 * @return     true if done charging and connected to charger
 */
bool furi_hal_power_is_charging_done();

/** Switch MCU to SHUTDOWN */
void furi_hal_power_shutdown();

/** Poweroff device
 */
void furi_hal_power_off();

/** Reset device
 */
void furi_hal_power_reset();

/** OTG enable
 */
void furi_hal_power_enable_otg();

/** OTG disable
 */
void furi_hal_power_disable_otg();

/** Check OTG status and disable it if falt happened
 */
void furi_hal_power_check_otg_status();

/** Get OTG status
 *
 * @return     true if enabled
 */
bool furi_hal_power_is_otg_enabled();

/** Get battery charge voltage limit in V
 *
 * @return     voltage in V
 */
float furi_hal_power_get_battery_charge_voltage_limit();

/** Set battery charge voltage limit in V
 *
 * Invalid values will be clamped downward to the nearest valid value.
 *
 * @param      voltage[in]  voltage in V
 *
 * @return     voltage in V
 */
void furi_hal_power_set_battery_charge_voltage_limit(float voltage);

/** Get remaining battery battery capacity in mAh
 *
 * @return     capacity in mAh
 */
uint32_t furi_hal_power_get_battery_remaining_capacity();

/** Get full charge battery capacity in mAh
 *
 * @return     capacity in mAh
 */
uint32_t furi_hal_power_get_battery_full_capacity();

/** Get battery capacity in mAh from battery profile
 *
 * @return     capacity in mAh
 */
uint32_t furi_hal_power_get_battery_design_capacity();

/** Get battery voltage in V
 *
 * @param      ic    FuriHalPowerIc to get measurment
 *
 * @return     voltage in V
 */
float furi_hal_power_get_battery_voltage(FuriHalPowerIC ic);

/** Get battery current in A
 *
 * @param      ic    FuriHalPowerIc to get measurment
 *
 * @return     current in A
 */
float furi_hal_power_get_battery_current(FuriHalPowerIC ic);

/** Get temperature in C
 *
 * @param      ic    FuriHalPowerIc to get measurment
 *
 * @return     temperature in C
 */
float furi_hal_power_get_battery_temperature(FuriHalPowerIC ic);

/** Get USB voltage in V
 *
 * @return     voltage in V
 */
float furi_hal_power_get_usb_voltage();

/** Enable 3.3v on external gpio and sd card
 */
void furi_hal_power_enable_external_3_3v();

/** Disable 3.3v on external gpio and sd card
 */
void furi_hal_power_disable_external_3_3v();

/** Enter supress charge mode.
 *
 * Use this function when your application need clean power supply.
 */
void furi_hal_power_suppress_charge_enter();

/** Exit supress charge mode
 */
void furi_hal_power_suppress_charge_exit();

/** Get power information
 *
 * @param[in]  callback     callback to provide with new data
 * @param[in]  sep          category separator character
 * @param[in]  context      context to pass to callback
 */
void furi_hal_power_info_get(PropertyValueCallback callback, char sep, void* context);

/** Get power debug information
 *
 * @param[in]  callback     callback to provide with new data
 * @param[in]  context      context to pass to callback
 */
void furi_hal_power_debug_get(PropertyValueCallback callback, void* context);

#ifdef __cplusplus
}
#endif
