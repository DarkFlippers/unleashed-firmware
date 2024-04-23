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
void furi_hal_debug_enable(void);

/** Disable MCU debug */
void furi_hal_debug_disable(void);

/** Check if GDB debug session is active */
bool furi_hal_debug_is_gdb_session_active(void);

#ifdef __cplusplus
}
#endif
