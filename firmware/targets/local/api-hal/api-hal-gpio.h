#pragma once
#include "main.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

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
void hal_gpio_init(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed);

// write value to GPIO, false = LOW, true = HIGH
void hal_gpio_write(const GpioPin* gpio, const bool state);

// read value from GPIO, false = LOW, true = HIGH
bool hal_gpio_read(const GpioPin* gpio);

void enable_cc1101_irq();

#ifdef __cplusplus
}
#endif
