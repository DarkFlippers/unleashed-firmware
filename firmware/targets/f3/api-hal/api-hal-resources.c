#include "main.h"
#include "flipper_v2.h"

const GpioPin input_gpio[GPIO_INPUT_PINS_COUNT] = {
    {BUTTON_UP_GPIO_Port, BUTTON_UP_Pin},
    {BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin},
    {BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin},
    {BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin},
    {BUTTON_OK_GPIO_Port, BUTTON_OK_Pin},
    {BUTTON_BACK_GPIO_Port, BUTTON_BACK_Pin},
};

const bool input_invert[GPIO_INPUT_PINS_COUNT] = {
    true, // {BUTTON_UP_GPIO_Port, BUTTON_UP_Pin},
    true, // {BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin},
    true, // {BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin},
    true, // {BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin},
    true, // {BUTTON_OK_GPIO_Port, BUTTON_OK_Pin},
    true, // {BUTTON_BACK_GPIO_Port, BUTTON_BACK_Pin},
};

const GpioPin led_gpio[3] = {
    {LED_RED_GPIO_Port, LED_RED_Pin},
    {LED_GREEN_GPIO_Port, LED_GREEN_Pin},
    {LED_BLUE_GPIO_Port, LED_BLUE_Pin}};

const GpioPin backlight_gpio = {DISPLAY_BACKLIGHT_GPIO_Port, DISPLAY_BACKLIGHT_Pin};
const GpioPin sd_cs_gpio = {SD_CS_GPIO_Port, SD_CS_Pin};
const GpioPin vibro_gpio = {VIBRO_GPIO_Port, VIBRO_Pin};
const GpioPin ibutton_gpio = {iBTN_GPIO_Port, iBTN_Pin};
const GpioPin cc1101_g0_gpio = {CC1101_G0_GPIO_Port, CC1101_G0_Pin};
