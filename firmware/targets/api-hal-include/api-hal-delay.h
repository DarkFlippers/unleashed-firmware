#pragma once
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Delay in milliseconds
 * @warning Cannot be used from ISR
 */
void delay(float milliseconds);

/** Delay in microseconds */
void delay_us(float microseconds);

/** Init DWT */
void delay_us_init_DWT(void);

#ifdef __cplusplus
}
#endif
