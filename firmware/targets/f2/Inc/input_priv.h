#ifndef __INPUT_PRIV_H
#define __INPUT_PRIV_H

#include "main.h"
#include "flipper_v2.h"

#define DEBOUNCE_TICKS 10

const GpioPin input_gpio[] = {
    {BUTTON_UP_GPIO_Port, BUTTON_UP_Pin},
    {BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin},
    {BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin},
    {BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin},
    {BUTTON_OK_GPIO_Port, BUTTON_OK_Pin},
    {BUTTON_BACK_GPIO_Port, BUTTON_BACK_Pin},
    {CHRG_GPIO_Port, CHRG_Pin}
};

const bool input_invert[] = {
    false, // {BUTTON_UP_GPIO_Port, BUTTON_UP_Pin},
    false, // {BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin},
    false, // {BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin},
    false, // {BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin},
    false, // {BUTTON_OK_GPIO_Port, BUTTON_OK_Pin},
    false, // {BUTTON_BACK_GPIO_Port, BUTTON_BACK_Pin},
    true, // {CHRG_GPIO_Port, CHRG_Pin}
};

#endif /* __INPUT_PRIV_H */
