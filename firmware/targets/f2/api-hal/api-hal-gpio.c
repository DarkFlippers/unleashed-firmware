#include "api-hal-gpio.h"

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

bool hal_gpio_read_sd_detect(void) {
    return true;
}

void enable_cc1101_irq() {
    
}
