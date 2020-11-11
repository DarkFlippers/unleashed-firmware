#pragma once

#include <stdint.h>
#include <stdbool.h>

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
