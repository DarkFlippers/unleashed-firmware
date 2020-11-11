#include "api-hal-gpio.h"
#include "api-hal-resources.h"

// init GPIO
void hal_gpio_init(GpioPin* gpio, GpioMode mode, GpioPull pull, GpioSpeed speed) {
    // TODO: Alternate Functions
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = gpio->pin;
    GPIO_InitStruct.Mode = mode;
    GPIO_InitStruct.Pull = pull;
    GPIO_InitStruct.Speed = speed;

    HAL_GPIO_Init(gpio->port, &GPIO_InitStruct);
}

bool hal_gpio_read_sd_detect(void) {
    bool result = false;
    // create pin
    GpioPin sd_cs_pin = sd_cs_gpio;
    // TODO open record
    GpioPin* sd_cs_record = &sd_cs_pin;

    // configure pin as input
    gpio_init_ex(sd_cs_record, GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
    delay(50);

    // if gpio_read == 0 return true else return false
    result = !gpio_read(sd_cs_record);

    // configure pin back
    gpio_init_ex(sd_cs_record, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    delay(50);

    return result;
}