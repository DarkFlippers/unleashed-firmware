#include <furi_hal.h>
#include <stm32wbxx_ll_utils.h>

void furi_hal_init() {
    furi_hal_i2c_init();
    furi_hal_light_init();
    furi_hal_spi_init();
    furi_hal_version_init();
}

void furi_hal_delay_ms(float milliseconds) {
    LL_mDelay((uint32_t)milliseconds);
}

void furi_hal_delay_us(float microseconds) {
    microseconds = microseconds / 1000;
    if(microseconds < 1) {
        microseconds = 1;
    }
    LL_mDelay((uint32_t)microseconds);
}
