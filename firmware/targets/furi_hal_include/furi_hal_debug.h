/**
 * @file furi_hal_debug.h
 * Debug HAL API
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Enable MCU debug */
void furi_hal_debug_enable();

/** Disable MCU debug */
void furi_hal_debug_disable();

#ifdef __cplusplus
}
#endif
