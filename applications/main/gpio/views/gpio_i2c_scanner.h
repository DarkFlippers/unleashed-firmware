#pragma once

#include <gui/view.h>
#include "../gpio_i2c_scanner_control.h"

typedef struct GpioI2CScanner GpioI2CScanner;
typedef void (*GpioI2CScannerOkCallback)(InputType type, void* context);

GpioI2CScanner* gpio_i2c_scanner_alloc();

void gpio_i2c_scanner_free(GpioI2CScanner* gpio_i2c_scanner);

View* gpio_i2c_scanner_get_view(GpioI2CScanner* gpio_i2c_scanner);

void gpio_i2c_scanner_set_ok_callback(
    GpioI2CScanner* gpio_i2c_scanner,
    GpioI2CScannerOkCallback callback,
    void* context);

void gpio_i2c_scanner_update_state(GpioI2CScanner* instance, I2CScannerState* st);
