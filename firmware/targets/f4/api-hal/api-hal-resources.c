#include <api-hal-resources.h>
#include "main.h"
#include <furi.h>

const InputPin input_pins[] = {
    {.port = BUTTON_UP_GPIO_Port, .pin = BUTTON_UP_Pin, .key = InputKeyUp, .inverted = true},
    {.port = BUTTON_DOWN_GPIO_Port,
     .pin = BUTTON_DOWN_Pin,
     .key = InputKeyDown,
     .inverted = true},
    {.port = BUTTON_RIGHT_GPIO_Port,
     .pin = BUTTON_RIGHT_Pin,
     .key = InputKeyRight,
     .inverted = true},
    {.port = BUTTON_LEFT_GPIO_Port,
     .pin = BUTTON_LEFT_Pin,
     .key = InputKeyLeft,
     .inverted = true},
    {.port = BUTTON_OK_GPIO_Port, .pin = BUTTON_OK_Pin, .key = InputKeyOk, .inverted = false},
    {.port = BUTTON_BACK_GPIO_Port,
     .pin = BUTTON_BACK_Pin,
     .key = InputKeyBack,
     .inverted = true},
};

const size_t input_pins_count = sizeof(input_pins) / sizeof(InputPin);

const GpioPin led_gpio[3] = {
    {LED_RED_GPIO_Port, LED_RED_Pin},
    {LED_GREEN_GPIO_Port, LED_GREEN_Pin},
    {LED_BLUE_GPIO_Port, LED_BLUE_Pin}};

const GpioPin backlight_gpio = {DISPLAY_BACKLIGHT_GPIO_Port, DISPLAY_BACKLIGHT_Pin};
const GpioPin sd_cs_gpio = {SD_CS_GPIO_Port, SD_CS_Pin};
const GpioPin vibro_gpio = {VIBRO_GPIO_Port, VIBRO_Pin};
const GpioPin ibutton_gpio = {iBTN_GPIO_Port, iBTN_Pin};
const GpioPin cc1101_g0_gpio = {CC1101_G0_GPIO_Port, CC1101_G0_Pin};
