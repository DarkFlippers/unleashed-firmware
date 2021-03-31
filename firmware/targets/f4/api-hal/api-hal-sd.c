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
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSdCard);

    // configure pin as input
    gpio_init_ex(device->chip_select, GpioModeInput, GpioPullUp, GpioSpeedVeryHigh);
    delay(1);

    // if gpio_read == 0 return true else return false
    result = !gpio_read(device->chip_select);

    // configure pin back
    gpio_init_ex(device->chip_select, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    gpio_write(device->chip_select, 1);
    delay(1);

    api_hal_spi_device_return(device);

    return result;
}