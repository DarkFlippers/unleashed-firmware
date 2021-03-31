#include <api-hal-gpio.h>
#include <api-hal-spi.h>
#include <api-hal-resources.h>
#include <api-hal-delay.h>

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
    #ifdef INVERT_RFID_IN
        return (HAL_COMP_GetOutputLevel(&hcomp1) == COMP_OUTPUT_LEVEL_LOW);
    #else
        return (HAL_COMP_GetOutputLevel(&hcomp1) == COMP_OUTPUT_LEVEL_HIGH);
    #endif
}
