/**
 * @file furi_hal_cortex.h
 * ARM Cortex HAL
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Early init stage for cortex
 */
void furi_hal_cortex_init_early();

/** Microseconds delay
 *
 * @param[in]  microseconds  The microseconds to wait
 */
void furi_hal_cortex_delay_us(uint32_t microseconds);

/** Get instructions per microsecond count
 *
 * @return     instructions per microsecond count
 */
uint32_t furi_hal_cortex_instructions_per_microsecond();

#ifdef __cplusplus
}
#endif
