#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <api-hal-resources.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Init light driver */
void api_hal_light_init();

/**
 * Set light value
 * @param light - Light
 * @param value - light brightness [0-255]
 */
void api_hal_light_set(Light light, uint8_t value);

#ifdef __cplusplus
}
#endif
