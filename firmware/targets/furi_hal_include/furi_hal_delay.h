/**
 * @file furi_hal_delay.h
 * Delay HAL API
 */

#pragma once

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t instructions_per_us;

/** Init DWT
 */
void furi_hal_delay_init(void);

/** Delay in milliseconds
 * @warning    Cannot be used from ISR
 *
 * @param[in]  milliseconds  milliseconds to wait
 */
void delay(float milliseconds);

/** Delay in microseconds
 *
 * @param[in]  microseconds  microseconds to wait
 */
void delay_us(float microseconds);

/** Get current millisecond
 * 
 * System uptime, pProvided by HAL, may overflow.
 *
 * @return     Current milliseconds
 */
uint32_t millis(void);

#ifdef __cplusplus
}
#endif
