#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <m-string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize drivers */
void api_hal_power_init();

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

/* Get battery voltage in V */
float api_hal_power_get_battery_voltage();

/* Get battery current in A */
float api_hal_power_get_battery_current();

/* Get power system component state */
void api_hal_power_dump_state(string_t buffer);

#ifdef __cplusplus
}
#endif
