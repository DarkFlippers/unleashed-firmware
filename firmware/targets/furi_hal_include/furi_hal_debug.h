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

/** Check if GDB debug session is active */
bool furi_hal_debug_is_gdb_session_active();

#ifdef __cplusplus
}
#endif
