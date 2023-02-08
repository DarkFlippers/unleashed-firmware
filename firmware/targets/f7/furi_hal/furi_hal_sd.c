#include <furi_hal_sd.h>
#include <stm32wbxx_ll_gpio.h>
#include <furi.h>
#include <furi_hal.h>

void hal_sd_detect_init(void) {
    // low speed input with pullup
    furi_hal_gpio_init(&gpio_sdcard_cd, GpioModeInput, GpioPullUp, GpioSpeedLow);
}

void hal_sd_detect_set_low(void) {
    // low speed input with pullup
    furi_hal_gpio_init_simple(&gpio_sdcard_cd, GpioModeOutputOpenDrain);
    furi_hal_gpio_write(&gpio_sdcard_cd, 0);
}

bool hal_sd_detect(void) {
    bool result = !furi_hal_gpio_read(&gpio_sdcard_cd);
    return result;
}

FuriHalSpiBusHandle* furi_hal_sd_spi_handle = NULL;
