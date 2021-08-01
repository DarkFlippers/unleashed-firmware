#include "decoder-gpio-out.h"
#include <furi.h>
#include <api-hal.h>

void DecoderGpioOut::process_front(bool polarity, uint32_t time) {
    hal_gpio_write(&gpio_ext_pa7, polarity);
}

DecoderGpioOut::DecoderGpioOut() {
    hal_gpio_init_simple(&gpio_ext_pa7, GpioModeOutputPushPull);
}

DecoderGpioOut::~DecoderGpioOut() {
    hal_gpio_init_simple(&gpio_ext_pa7, GpioModeAnalog);
}
