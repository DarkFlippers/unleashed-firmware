#include "GPIO_reader_item.h"

const char* gpio_item_get_pin_name(uint8_t index) {
    furi_assert(index < GPIO_ITEM_COUNT);
    return gpio_item[index].name;
}

const char* gpio_item_get_pull_mode(uint8_t pull_mode) {
    furi_assert(pull_mode < GPIO_PULL_COUNT);
    return gpio_pull_mode[pull_mode].name;
}

const char* gpio_item_get_pin_level(uint8_t index) {
    furi_assert(index < GPIO_ITEM_COUNT);
    //furi_hal_gpio_write(gpio_item[index].pin, level);
    if(furi_hal_gpio_read(gpio_item[index].pin)) {
        return "High";
    } else {
        return "Low";
    }
}

void gpio_item_configure_pin(uint8_t index, uint8_t pull_mode) {
    furi_assert(index < GPIO_ITEM_COUNT);
    furi_hal_gpio_init(
        gpio_item[index].pin, GpioModeInput, gpio_pull_mode[pull_mode].pull, GpioSpeedVeryHigh);
}