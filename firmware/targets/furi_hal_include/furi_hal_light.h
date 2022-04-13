/**
 * @file furi_hal_light.h
 * Light control HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi_hal_resources.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Init light driver
 */
void furi_hal_light_init();

/** Set light value
 *
 * @param      light  Light
 * @param      value  light brightness [0-255]
 */
void furi_hal_light_set(Light light, uint8_t value);

/** Execute sequence
 *
 * @param      sequence  Sequence to execute
 */
void furi_hal_light_sequence(const char* sequence);

#ifdef __cplusplus
}
#endif
