#include <api-hal-gpio.h>
#include <api-hal-version.h>

// init GPIO
void hal_gpio_init(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed) {
    // TODO: Alternate Functions
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = gpio->pin;
    GPIO_InitStruct.Mode = mode;
    GPIO_InitStruct.Pull = pull;
    GPIO_InitStruct.Speed = speed;

    HAL_GPIO_Init(gpio->port, &GPIO_InitStruct);
}

extern COMP_HandleTypeDef hcomp1;

bool get_rfid_in_level() {
    bool value = false;
    if (api_hal_version_get_hw_version() > 7) {
        value = (HAL_COMP_GetOutputLevel(&hcomp1) == COMP_OUTPUT_LEVEL_LOW);
    } else {
        value = (HAL_COMP_GetOutputLevel(&hcomp1) == COMP_OUTPUT_LEVEL_HIGH);
    }

#ifdef INVERT_RFID_IN
    return !value;
#else
    return value;
#endif
}
