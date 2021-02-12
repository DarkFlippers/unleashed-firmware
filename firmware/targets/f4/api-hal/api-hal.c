#include <api-hal.h>

void api_hal_init() {
    api_hal_os_init();
    api_hal_vcp_init();
    api_hal_spi_init();
}