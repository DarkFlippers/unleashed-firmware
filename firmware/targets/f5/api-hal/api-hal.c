#include <api-hal.h>

void api_hal_init() {
    api_hal_delay_init();
    api_hal_os_init();
    api_hal_vcp_init();
    api_hal_spi_init();
    api_hal_i2c_init();
    api_hal_power_init();
    api_hal_light_init();
    api_hal_vibro_init();
    api_hal_subghz_init();
}
