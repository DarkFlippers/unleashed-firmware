#include <api-hal.h>

void api_hal_init() {
    api_hal_delay_init();
    FURI_LOG_I("FURI_HAL", "DELAY OK");
    api_hal_os_init();
    FURI_LOG_I("FURI_HAL", "OS OK");
    api_hal_vcp_init();
    FURI_LOG_I("FURI_HAL", "VCP OK");
    api_hal_spi_init();
    FURI_LOG_I("FURI_HAL", "SPI OK");
    api_hal_i2c_init();
    FURI_LOG_I("FURI_HAL", "I2C OK");
    api_hal_power_init();
    FURI_LOG_I("FURI_HAL", "POWER OK");
    api_hal_light_init();
    FURI_LOG_I("FURI_HAL", "LIGHT OK");
    api_hal_vibro_init();
    FURI_LOG_I("FURI_HAL", "VIBRO OK");
    api_hal_subghz_init();
    FURI_LOG_I("FURI_HAL", "SUBGHZ OK");
}
