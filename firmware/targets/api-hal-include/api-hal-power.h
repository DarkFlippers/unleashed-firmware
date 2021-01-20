#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <m-string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ApiHalPowerICCharger,
    ApiHalPowerICFuelGauge,
} ApiHalPowerIC;

/* Initialize drivers */
void api_hal_power_init();

/* Go to deep sleep */
void api_hal_power_deep_sleep();

/* Get predicted remaining battery capacity in percents */
uint8_t api_hal_power_get_pct();

/* Get charging status */
bool api_hal_power_is_charging();

/* Poweroff system */
void api_hal_power_off();

/* OTG enable */
void api_hal_power_enable_otg();

/* OTG disable */
void api_hal_power_disable_otg();

/* Get remaining battery battery capacity in mAh */
uint32_t api_hal_power_get_battery_remaining_capacity();

/* Get full charge battery capacity in mAh */
uint32_t api_hal_power_get_battery_full_capacity();

/* Get battery voltage in V */
float api_hal_power_get_battery_voltage(ApiHalPowerIC ic);

/* Get battery current in A */
float api_hal_power_get_battery_current(ApiHalPowerIC ic);

/* Get temperature in C */
float api_hal_power_get_battery_temperature(ApiHalPowerIC ic);

/* Get System voltage in V */
float api_hal_power_get_system_voltage();

/* Get USB voltage in V */
float api_hal_power_get_usb_voltage();

/* Get power system component state */
void api_hal_power_dump_state(string_t buffer);

#ifdef __cplusplus
}
#endif
