#include <api-hal.h>

void api_hal_init() {
    api_hal_i2c_init();
    api_hal_light_init();
}