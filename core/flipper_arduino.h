#pragma once
#include "stdint.h"

// Arduino defines
#define pinMode gpio_init
#define digitalWrite gpio_write
#define digitalRead gpio_read
#define delayMicroseconds delay_us
#define delay osDelay

#define OUTPUT GpioModeOutputPushPull
#define INPUT GpioModeInput
#define LOW false
#define HIGH true

typedef uint8_t byte;