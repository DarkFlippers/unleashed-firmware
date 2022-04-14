/**
 * @file furi_hal_delay.h
 * Delay HAL API
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Init Delay subsystem */
void furi_hal_delay_init();

/** Get instructions per microsecond count */
uint32_t furi_hal_delay_instructions_per_microsecond();

/** Increase tick counter.
 *  Should be called from SysTick ISR
 */
void furi_hal_tick(void);

/** Get current tick counter
 *
 * System uptime, may overflow.
 *
 * @return     Current ticks in milliseconds
 */
uint32_t furi_hal_get_tick(void);

/** Convert milliseconds to ticks
 *
 * @param[in]   milliseconds    time in milliseconds
 * @return      time in ticks
 */
uint32_t furi_hal_ms_to_ticks(float milliseconds);

/** Delay in milliseconds
 * @warning    Cannot be used from ISR
 *
 * @param[in]  milliseconds  milliseconds to wait
 */
void furi_hal_delay_ms(float milliseconds);

/** Delay in microseconds
 *
 * @param[in]  microseconds  microseconds to wait
 */
void furi_hal_delay_us(float microseconds);

#ifdef __cplusplus
}
#endif
