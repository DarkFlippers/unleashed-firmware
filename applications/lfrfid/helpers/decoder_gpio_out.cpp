#include "decoder_gpio_out.h"
#include <furi.h>
#include <furi_hal.h>

void DecoderGpioOut::process_front(bool polarity, uint32_t /* time */) {
    furi_hal_gpio_write(&gpio_ext_pa7, polarity);
}

DecoderGpioOut::DecoderGpioOut() {
    furi_hal_gpio_init_simple(&gpio_ext_pa7, GpioModeOutputPushPull);
}

DecoderGpioOut::~DecoderGpioOut() {
    furi_hal_gpio_init_simple(&gpio_ext_pa7, GpioModeAnalog);
}
