#pragma once
#include "main.h"
#include "stdbool.h"

// hw-api

typedef char GPIO_TypeDef;

typedef enum {
    GpioModeInput,
    GpioModeOutputPushPull,
    GpioModeOutputOpenDrain,
    GpioModeAltFunctionPushPull,
    GpioModeAltFunctionOpenDrain,
    GpioModeAnalog,
    GpioModeInterruptRise,
    GpioModeInterruptFall,
    GpioModeInterruptRiseFall,
    GpioModeEventRise,
    GpioModeEventFall,
    GpioModeEventRiseFall,
} GpioMode;

typedef enum {
    GpioSpeedLow,
    GpioSpeedMedium,
    GpioSpeedHigh,
    GpioSpeedVeryHigh,
} GpioSpeed;

typedef enum {
    GpioPullNo,
    GpioPullUp,
    GpioPullDown,
} GpioPull;

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} GpioPin;

// init GPIO
void hal_gpio_init(GpioPin* gpio, GpioMode mode, GpioPull pull, GpioSpeed speed);

// write value to GPIO, false = LOW, true = HIGH
void hal_gpio_write(GpioPin* gpio, bool state);

// read value from GPIO, false = LOW, true = HIGH
bool hal_gpio_read(const GpioPin* gpio);