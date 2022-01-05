#pragma once

#include <furi_hal_gpio.h>

#define GPIO_ITEM_COUNT 8

void gpio_item_configure_pin(uint8_t index, GpioMode mode);

void gpio_item_configure_all_pins(GpioMode mode);

void gpio_item_set_pin(uint8_t index, bool level);

void gpio_item_set_all_pins(bool level);

const char* gpio_item_get_pin_name(uint8_t index);
