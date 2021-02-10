#pragma once
#include "main.h"
#include <furi.h>

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

typedef struct {
    const GPIO_TypeDef* port;
    const uint16_t pin;
    const InputKey key;
    const bool inverted;
} InputPin;

extern const InputPin input_pins[];
extern const size_t input_pins_count;

extern const GpioPin led_gpio[3];
extern const GpioPin backlight_gpio;
extern const GpioPin sd_cs_gpio;
extern const GpioPin vibro_gpio;
extern const GpioPin ibutton_gpio;
extern const GpioPin cc1101_g0_gpio;
