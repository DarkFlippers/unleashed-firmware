#include <furi-hal.h>

void furi_hal_init() {
    furi_hal_i2c_init();
    furi_hal_light_init();
    furi_hal_spi_init();
}