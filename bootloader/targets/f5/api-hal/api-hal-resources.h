#pragma once

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

#ifdef __cplusplus
}
#endif
