#include "main.h"
#include "flipper_v2.h"

const bool input_invert[GPIO_INPUT_PINS_COUNT] = {
    true, // {BUTTON_UP_GPIO_Port, BUTTON_UP_Pin},
    true, // {BUTTON_DOWN_GPIO_Port, BUTTON_DOWN_Pin},
    true, // {BUTTON_RIGHT_GPIO_Port, BUTTON_RIGHT_Pin},
    true, // {BUTTON_LEFT_GPIO_Port, BUTTON_LEFT_Pin},
    false, // {BUTTON_OK_GPIO_Port, BUTTON_OK_Pin},
    true, // {BUTTON_BACK_GPIO_Port, BUTTON_BACK_Pin},
};
