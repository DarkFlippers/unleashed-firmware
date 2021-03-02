#pragma once

#include "main.h"
#include <furi.h>

#include <stm32wbxx.h>
#include <stm32wbxx_ll_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_SCL_Pin LL_GPIO_PIN_9
#define I2C_SCL_GPIO_Port GPIOA
#define I2C_SDA_Pin LL_GPIO_PIN_10
#define I2C_SDA_GPIO_Port GPIOA

#define POWER_I2C I2C1

/* Input Related Constants */
#define INPUT_DEBOUNCE_TICKS 20

/* Input Keys */
typedef enum {
    InputKeyUp,
    InputKeyDown,
    InputKeyRight,
    InputKeyLeft,
    InputKeyOk,
    InputKeyBack,
} InputKey;

/* Light */
typedef enum {
    LightRed,
    LightGreen,
    LightBlue,
    LightBacklight,
} Light;

typedef struct {
    const GPIO_TypeDef* port;
    const uint16_t pin;
    const InputKey key;
    const bool inverted;
} InputPin;

extern const InputPin input_pins[];
extern const size_t input_pins_count;

extern const GpioPin sd_cs_gpio;
extern const GpioPin vibro_gpio;
extern const GpioPin ibutton_gpio;
extern const GpioPin cc1101_g0_gpio;

#ifdef __cplusplus
}
#endif
