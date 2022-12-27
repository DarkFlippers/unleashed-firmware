#ifndef GPIO_READER_ITEM
#define GPIO_READER_ITEM

#include <furi.h>
#include <furi_hal_resources.h>

#define GPIO_ITEM_COUNT 8
#define GPIO_PULL_COUNT 3

typedef struct {
    const char* name;
    const GpioPin* pin;
} GpioItem;

static const GpioItem gpio_item[GPIO_ITEM_COUNT] = {
    {"2: PA7", &gpio_ext_pa7},
    {"3: PA6", &gpio_ext_pa6},
    {"4: PA4", &gpio_ext_pa4},
    {"5: PB3", &gpio_ext_pb3},
    {"6: PB2", &gpio_ext_pb2},
    {"7: PC3", &gpio_ext_pc3},
    {"15: PC1", &gpio_ext_pc1},
    {"16: PC0", &gpio_ext_pc0},
};

typedef struct {
    const char* name;
    const GpioPull pull;
} GpioPullMode;

static const GpioPullMode gpio_pull_mode[3] = {
    {"high impedence", GpioPullNo},
    {"pull up", GpioPullUp},
    {"pull down", GpioPullDown},
};

const char* gpio_item_get_pin_name(uint8_t index);
const char* gpio_item_get_pin_level(uint8_t index);
void gpio_item_configure_pin(uint8_t index, uint8_t pullMode);
const char* gpio_item_get_pull_mode(uint8_t pull_mode);

#endif