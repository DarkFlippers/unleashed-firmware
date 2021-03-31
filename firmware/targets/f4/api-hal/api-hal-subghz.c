#include "api-hal-subghz.h"
#include <api-hal-spi.h>
#include <cc1101.h>

void api_hal_subghz_init() {
    const ApiHalSpiDevice* device = api_hal_spi_device_get(ApiHalSpiDeviceIdSubGhz);
    cc1101_init(device);
    api_hal_spi_device_return(device);
}

void api_hal_subghz_band_set(RfBand band) {}