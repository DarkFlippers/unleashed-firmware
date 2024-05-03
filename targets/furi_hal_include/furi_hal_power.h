/**
 * @file furi_hal_power.h
 * Power HAL API
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <core/string.h>
#include <core/common_defines.h>
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
void furi_hal_power_init(void);

/** Check if gauge is ok
 * 
 * Verifies that:
 * - gauge is alive
 * - correct profile loaded
 * - self diagnostic status is good
 * 
 * @return true if gauge is ok
 */
bool furi_hal_power_gauge_is_ok(void);

/** Check if gauge requests system shutdown
 * 
 * @return true if system shutdown requested
 */
bool furi_hal_power_is_shutdown_requested(void);

/** Get current insomnia level
 *
 * @return     insomnia level: 0 - no insomnia, >0 - insomnia, bearer count.
 */
uint16_t furi_hal_power_insomnia_level(void);

/** Enter insomnia mode Prevents device from going to sleep
 * @warning    Internally increases insomnia level Must be paired with
 *             furi_hal_power_insomnia_exit
 */
void furi_hal_power_insomnia_enter(void);

/** Exit insomnia mode Allow device to go to sleep
 * @warning    Internally decreases insomnia level. Must be paired with
 *             furi_hal_power_insomnia_enter
 */
void furi_hal_power_insomnia_exit(void);

/** Check if sleep available
 *
 * @return     true if available
 */
bool furi_hal_power_sleep_available(void);

/** Go to sleep
 */
void furi_hal_power_sleep(void);

/** Get predicted remaining battery capacity in percents
 *
 * @return     remaining battery capacity in percents
 */
uint8_t furi_hal_power_get_pct(void);

/** Get battery health state in percents
 *
 * @return     health in percents
 */
uint8_t furi_hal_power_get_bat_health_pct(void);

/** Get charging status
 *
 * @return     true if charging
 */
bool furi_hal_power_is_charging(void);

/** Get charge complete status
 *
 * @return     true if done charging and connected to charger
 */
bool furi_hal_power_is_charging_done(void);

/** Switch MCU to SHUTDOWN */
void furi_hal_power_shutdown(void);

/** Poweroff device
 */
void furi_hal_power_off(void);

/** Reset device
 */
FURI_NORETURN void furi_hal_power_reset(void);

/** OTG enable
 */
bool furi_hal_power_enable_otg(void);

/** OTG disable
 */
void furi_hal_power_disable_otg(void);

/** Check OTG status fault
 */
bool furi_hal_power_check_otg_fault(void);

/** Check OTG status and disable it if falt happened
 */
void furi_hal_power_check_otg_status(void);

/** Get OTG status
 *
 * @return     true if enabled
 */
bool furi_hal_power_is_otg_enabled(void);

/** Get battery charge voltage limit in V
 *
 * @return     voltage in V
 */
float furi_hal_power_get_battery_charge_voltage_limit(void);

/** Set battery charge voltage limit in V
 *
 * Invalid values will be clamped downward to the nearest valid value.
 *
 * @param[in]      voltage  voltage in V
 */
void furi_hal_power_set_battery_charge_voltage_limit(float voltage);

/** Get remaining battery battery capacity in mAh
 *
 * @return     capacity in mAh
 */
uint32_t furi_hal_power_get_battery_remaining_capacity(void);

/** Get full charge battery capacity in mAh
 *
 * @return     capacity in mAh
 */
uint32_t furi_hal_power_get_battery_full_capacity(void);

/** Get battery capacity in mAh from battery profile
 *
 * @return     capacity in mAh
 */
uint32_t furi_hal_power_get_battery_design_capacity(void);

/** Get battery voltage in V
 *
 * @param[in]      ic    FuriHalPowerIc to get measurment
 *
 * @return     voltage in V
 */
float furi_hal_power_get_battery_voltage(FuriHalPowerIC ic);

/** Get battery current in A
 *
 * @param[in]      ic    FuriHalPowerIc to get measurment
 *
 * @return     current in A
 */
float furi_hal_power_get_battery_current(FuriHalPowerIC ic);

/** Get temperature in C
 *
 * @param[in]      ic    FuriHalPowerIc to get measurment
 *
 * @return     temperature in C
 */
float furi_hal_power_get_battery_temperature(FuriHalPowerIC ic);

/** Get USB voltage in V
 *
 * @return     voltage in V
 */
float furi_hal_power_get_usb_voltage(void);

/** Enable 3.3v on external gpio and sd card
 */
void furi_hal_power_enable_external_3_3v(void);

/** Disable 3.3v on external gpio and sd card
 */
void furi_hal_power_disable_external_3_3v(void);

/** Enter supress charge mode.
 *
 * Use this function when your application need clean power supply.
 */
void furi_hal_power_suppress_charge_enter(void);

/** Exit supress charge mode
 */
void furi_hal_power_suppress_charge_exit(void);

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
