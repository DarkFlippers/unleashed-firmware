#pragma once

#include <gui/view.h>
#include "../gpio_i2c_sfp_control.h"

typedef struct GpioI2CSfp GpioI2CSfp;
typedef void (*GpioI2CSfpOkCallback)(InputType type, void* context);

GpioI2CSfp* gpio_i2c_sfp_alloc();

void gpio_i2c_sfp_free(GpioI2CSfp* gpio_i2c_sfp);

View* gpio_i2c_sfp_get_view(GpioI2CSfp* gpio_i2c_sfp);

void gpio_i2c_sfp_set_ok_callback(
    GpioI2CSfp* gpio_i2c_sfp,
    GpioI2CSfpOkCallback callback,
    void* context);

void gpio_i2c_sfp_update_state(GpioI2CSfp* instance, I2CSfpState* st);
