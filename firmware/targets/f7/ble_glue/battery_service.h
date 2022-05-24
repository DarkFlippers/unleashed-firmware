#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void battery_svc_start();

void battery_svc_stop();

bool battery_svc_is_started();

bool battery_svc_update_level(uint8_t battery_level);

bool battery_svc_update_power_state();

#ifdef __cplusplus
}
#endif
