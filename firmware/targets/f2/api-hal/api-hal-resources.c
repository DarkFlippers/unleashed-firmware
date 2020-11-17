#include "main.h"
#include "flipper_v2.h"

#define DEBOUNCE_TICKS 10

const GpioPin input_gpio[GPIO_INPUT_PINS_COUNT] = {
    {BUTTON_UP_GPIO_Port, BUTTON_UP_Pin},
    {BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin},
    {BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin},
    {BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin},
    {BUTTON_OK_GPIO_Port, BUTTON_OK_Pin},
    {BUTTON_BACK_GPIO_Port, BUTTON_BACK_Pin},
    {CHRG_GPIO_Port, CHRG_Pin}};

const bool input_invert[GPIO_INPUT_PINS_COUNT] = {
    false, // {BUTTON_UP_GPIO_Port, BUTTON_UP_Pin},
    false, // {BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin},
    false, // {BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin},
    false, // {BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin},
    false, // {BUTTON_OK_GPIO_Port, BUTTON_OK_Pin},
    false, // {BUTTON_BACK_GPIO_Port, BUTTON_BACK_Pin},
    true, // {CHRG_GPIO_Port, CHRG_Pin}
};

const GpioPin led_gpio[3] = {
    {LED_RED_GPIO_Port, LED_RED_Pin},
    {LED_GREEN_GPIO_Port, LED_GREEN_Pin},
    {LED_BLUE_GPIO_Port, LED_BLUE_Pin}};

const GpioPin backlight_gpio = {DISPLAY_BACKLIGHT_GPIO_Port, DISPLAY_BACKLIGHT_Pin};
const GpioPin vibro_gpio = {VIBRO_GPIO_Port, VIBRO_Pin};
const GpioPin ibutton_gpio = {iButton_GPIO_Port, iButton_Pin};
