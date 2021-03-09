#include <api-hal-sd.h>
#include <api-hal-spi.h>
#include <api-hal-resources.h>
#include <api-hal-delay.h>
#include <furi.h>

void hal_sd_detect_init(void) {
    // nothing to do
}

void hal_sd_detect_set_low(void) {
    // nothing to do
}

bool hal_sd_detect(void) {
    bool result = false;

    // TODO open record
    const GpioPin* sd_cs_record = &sd_cs_gpio;

    // TODO: SPI manager
    api_hal_spi_lock(sd_fast_spi.spi);

    // configure pin as input
    gpio_init_ex(sd_cs_record, GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
    delay(1);

    // if gpio_read == 0 return true else return false
    result = !gpio_read(sd_cs_record);

    // configure pin back
    gpio_init_ex(sd_cs_record, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    gpio_write(sd_cs_record, 1);
    delay(1);

    // TODO: SPI manager
    api_hal_spi_unlock(sd_fast_spi.spi);

    return result;
}