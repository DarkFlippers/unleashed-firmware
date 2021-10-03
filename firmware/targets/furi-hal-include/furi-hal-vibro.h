/**
 * @file furi-hal-vibro.h
 * Vibro HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi-hal-resources.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize vibro
 */
void furi_hal_vibro_init();

/** Turn on/off vibro
 *
 * @param[in]  value  new state
 */
void furi_hal_vibro_on(bool value);

#ifdef __cplusplus
}
#endif
