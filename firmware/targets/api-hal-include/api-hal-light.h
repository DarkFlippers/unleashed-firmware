#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <api-hal-resources.h>

#ifdef __cplusplus
extern "C" {
#endif

void api_hal_light_init();

void api_hal_light_set(Light light, uint8_t value);

#ifdef __cplusplus
}
#endif
