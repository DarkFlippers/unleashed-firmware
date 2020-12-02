#pragma once
#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

void delay(float milliseconds);
void delay_us(float microseconds);
void delay_us_init_DWT(void);

#ifdef __cplusplus
}
#endif
