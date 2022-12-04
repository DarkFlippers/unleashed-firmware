#include "gpio_item.h"

#include <furi_hal_resources.h>

typedef struct {
    const char* name;
    const GpioPin* pin;
} GpioItem;

static const GpioItem gpio_item[GPIO_ITEM_COUNT] = {
    {"1.2: PA7", &gpio_ext_pa7},
    {"1.3: PA6", &gpio_ext_pa6},
    {"1.4: PA4", &gpio_ext_pa4},
    {"1.5: PB3", &gpio_ext_pb3},
    {"1.6: PB2", &gpio_ext_pb2},
    {"1.7: PC3", &gpio_ext_pc3},
    {"2.7: PC1", &gpio_ext_pc1},
    {"2.8: PC0", &gpio_ext_pc0},
};

void gpio_item_configure_pin(uint8_t index, GpioMode mode) {
    furi_assert(index < GPIO_ITEM_COUNT);
    furi_hal_gpio_write(gpio_item[index].pin, false);
    furi_hal_gpio_init(gpio_item[index].pin, mode, GpioPullNo, GpioSpeedVeryHigh);
}

void gpio_item_configure_all_pins(GpioMode mode) {
    for(uint8_t i = 0; i < GPIO_ITEM_COUNT; i++) {
        gpio_item_configure_pin(i, mode);
    }
}

void gpio_item_set_pin(uint8_t index, bool level) {
    furi_assert(index < GPIO_ITEM_COUNT);
    furi_hal_gpio_write(gpio_item[index].pin, level);
}

void gpio_item_set_all_pins(bool level) {
    for(uint8_t i = 0; i < GPIO_ITEM_COUNT; i++) {
        gpio_item_set_pin(i, level);
    }
}

const char* gpio_item_get_pin_name(uint8_t index) {
    furi_assert(index < GPIO_ITEM_COUNT + 1);
    if(index == GPIO_ITEM_COUNT) {
        return "ALL";
    } else {
        return gpio_item[index].name;
    }
}
