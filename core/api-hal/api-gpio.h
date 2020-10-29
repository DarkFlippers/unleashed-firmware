#pragma once
#include "flipper.h"
#include "flipper_v2.h"
#include "api-hal-gpio.h"

typedef struct {
    ValueMutex* gpio_mutex;
    GpioPin* gpio;
} GpioDisableRecord;

// init GPIO API
bool gpio_api_init();

// init GPIO
void gpio_init(GpioPin* gpio, GpioMode mode);

// init GPIO, extended version
void gpio_init_ex(GpioPin* gpio, GpioMode mode, GpioPull pull, GpioSpeed speed);

// write value to GPIO, false = LOW, true = HIGH
static inline void gpio_write(GpioPin* gpio, bool state) {
    hal_gpio_write(gpio, state);
}

// read value from GPIO, false = LOW, true = HIGH
static inline bool gpio_read(const GpioPin* gpio) {
    return hal_gpio_read(gpio);
}

// put GPIO to Z-state
void gpio_disable(GpioDisableRecord* gpio_record);

// get GPIO record
ValueMutex* gpio_open_mutex(const char* name);

// get GPIO record and acquire mutex
GpioPin* gpio_open(const char* name);