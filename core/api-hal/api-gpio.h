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

/**
 * Init GPIO API
 * @return true on successful gpio initialization, false otherwize
 */
bool gpio_api_init();

/**
 * Init GPIO
 * @param gpio GpioPin instance
 * @param mode GpioMode gpio mode
 */
void gpio_init(const GpioPin* gpio, const GpioMode mode);

/**
 * Init GPIO, extended version
 * @param gpio GpioPin instance
 * @param mode GpioMode gpio mode
 * @param pull GpioPull gpio pull mode
 * @param speed GpioSpeed gpio speed
 */
void gpio_init_ex(
    const GpioPin* gpio,
    const GpioMode mode,
    const GpioPull pull,
    const GpioSpeed speed);

/**
 * Write value to GPIO
 * @param gpio GpioPin instance
 * @param state false = LOW, true = HIGH
 */
static inline void gpio_write(const GpioPin* gpio, const bool state) {
    hal_gpio_write(gpio, state);
}

/**
 * Read value from GPIO
 * @param gpio GpioPin instance
 * @return false = LOW, true = HIGH
 */
static inline bool gpio_read(const GpioPin* gpio) {
    return hal_gpio_read(gpio);
}

/**
 * Put GPIO to Z-state
 * @param gpio_record GpioDisableRecord instance
 */
void gpio_disable(GpioDisableRecord* gpio_record);

/**
 * Get GPIO record
 * @param name name of record
 * @return ValueMutex instance
 */
ValueMutex* gpio_open_mutex(const char* name);

/**
 * Get GPIO record and acquire mutex
 * @param name name of record
 * @return GpioPin instance
 */
GpioPin* gpio_open(const char* name);

/**
 * Get RFID IN level
 * @return false = LOW, true = HIGH
 */
bool get_rfid_in_level();

#ifdef __cplusplus
}
#endif
