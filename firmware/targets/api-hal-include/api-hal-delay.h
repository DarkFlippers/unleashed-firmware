#pragma once
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Init DWT */
void api_hal_delay_init(void);

/**
 * Delay in milliseconds
 * @warning Cannot be used from ISR
 */
void delay(float milliseconds);

/** Delay in microseconds */
void delay_us(float microseconds);

#ifdef __cplusplus
}
#endif
