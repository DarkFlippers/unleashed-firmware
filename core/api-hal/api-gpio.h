#pragma once

#include "api-hal-gpio.h"
#include <furi/valuemutex.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ValueMutex* gpio_mutex;
    GpioPin* gpio;
} GpioDisableRecord;

// init GPIO API
bool gpio_api_init();

// init GPIO
void gpio_init(const GpioPin* gpio, const GpioMode mode);

// init GPIO, extended version
void gpio_init_ex(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed);

// write value to GPIO, false = LOW, true = HIGH
static inline void gpio_write(const GpioPin* gpio, const bool state) {
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

// get RFID IN level
bool get_rfid_in_level();

#ifdef __cplusplus
}
#endif
