#pragma once
#include "main.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

// this defined in xx_hal_gpio.c, so...
#define GPIO_NUMBER (16U)

typedef enum {
    GpioModeInput = GPIO_MODE_INPUT,
    GpioModeOutputPushPull = GPIO_MODE_OUTPUT_PP,
    GpioModeOutputOpenDrain = GPIO_MODE_OUTPUT_OD,
    GpioModeAltFunctionPushPull = GPIO_MODE_AF_PP,
    GpioModeAltFunctionOpenDrain = GPIO_MODE_AF_OD,
    GpioModeAnalog = GPIO_MODE_ANALOG,
    GpioModeInterruptRise = GPIO_MODE_IT_RISING,
    GpioModeInterruptFall = GPIO_MODE_IT_FALLING,
    GpioModeInterruptRiseFall = GPIO_MODE_IT_RISING_FALLING,
    GpioModeEventRise = GPIO_MODE_EVT_RISING,
    GpioModeEventFall = GPIO_MODE_EVT_FALLING,
    GpioModeEventRiseFall = GPIO_MODE_EVT_RISING_FALLING,
} GpioMode;

typedef enum {
    GpioSpeedLow = GPIO_SPEED_FREQ_LOW,
    GpioSpeedMedium = GPIO_SPEED_FREQ_MEDIUM,
    GpioSpeedHigh = GPIO_SPEED_FREQ_HIGH,
    GpioSpeedVeryHigh = GPIO_SPEED_FREQ_VERY_HIGH,
} GpioSpeed;

typedef enum {
    GpioPullNo = GPIO_NOPULL,
    GpioPullUp = GPIO_PULLUP,
    GpioPullDown = GPIO_PULLDOWN,
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
static inline void hal_gpio_write(const GpioPin* gpio, const bool state) {
    // writing to BSSR is an atomic operation
    if(state == true) {
        gpio->port->BSRR = gpio->pin;
    } else {
        gpio->port->BSRR = (uint32_t)gpio->pin << GPIO_NUMBER;
    }
}

// read value from GPIO, false = LOW, true = HIGH
static inline bool hal_gpio_read(const GpioPin* gpio) {
    if((gpio->port->IDR & gpio->pin) != 0x00U) {
        return true;
    } else {
        return false;
    }
}

void enable_cc1101_irq();

#ifdef __cplusplus
}
#endif
