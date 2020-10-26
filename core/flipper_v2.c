#include "flipper_v2.h"

bool init_flipper_api(void) {
    return gpio_api_init();
}