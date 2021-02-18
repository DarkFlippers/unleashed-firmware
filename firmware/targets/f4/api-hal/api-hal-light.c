#include <api-hal-light.h>
#include <stm32wbxx_ll_gpio.h>

// LCD backlight
#define BOOT_LCD_BL_PORT GPIOA
#define BOOT_LCD_BL_PIN LL_GPIO_PIN_15
// LEDs
#define LED_RED_PORT GPIOA
#define LED_RED_PIN LL_GPIO_PIN_1
#define LED_GREEN_PORT GPIOA
#define LED_GREEN_PIN LL_GPIO_PIN_2
#define LED_BLUE_PORT GPIOA
#define LED_BLUE_PIN LL_GPIO_PIN_3

static void api_hal_light_gpio_set(GPIO_TypeDef* port, uint32_t pin, uint8_t value) {
    if (value) {
        LL_GPIO_ResetOutputPin(port, pin);
    } else {
        LL_GPIO_SetOutputPin(port, pin);
    }
}

void api_hal_light_init() {
    LL_GPIO_SetPinMode(BOOT_LCD_BL_PORT, BOOT_LCD_BL_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(BOOT_LCD_BL_PORT, BOOT_LCD_BL_PIN, LL_GPIO_SPEED_FREQ_LOW);
    LL_GPIO_SetPinOutputType(BOOT_LCD_BL_PORT, BOOT_LCD_BL_PIN, LL_GPIO_OUTPUT_PUSHPULL);

    LL_GPIO_SetPinMode(LED_RED_PORT, LED_RED_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(LED_RED_PORT, LED_RED_PIN, LL_GPIO_OUTPUT_OPENDRAIN);

    LL_GPIO_SetPinMode(LED_GREEN_PORT, LED_GREEN_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(LED_GREEN_PORT, LED_GREEN_PIN, LL_GPIO_OUTPUT_OPENDRAIN);

    LL_GPIO_SetPinMode(LED_BLUE_PORT, LED_BLUE_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinOutputType(LED_BLUE_PORT, LED_BLUE_PIN, LL_GPIO_OUTPUT_OPENDRAIN);
}

void api_hal_light_set(Light light, uint8_t value) {
    switch(light) {
        case LightRed:
            api_hal_light_gpio_set(LED_RED_PORT, LED_RED_PIN, value);
        break;
        case LightGreen:
            api_hal_light_gpio_set(LED_GREEN_PORT, LED_GREEN_PIN, value);
        break;
        case LightBlue:
            api_hal_light_gpio_set(LED_BLUE_PORT, LED_BLUE_PIN, value);
        break;
        case LightBacklight:
            api_hal_light_gpio_set(BOOT_LCD_BL_PORT, BOOT_LCD_BL_PIN, !value);
        break;
        default:
        break;
    }
}