#include <furi.h>
#include <api-hal.h>

void rgb_set(bool r, bool g, bool b) {
    api_hal_light_set(LightRed, r ? 0xFF : 0x00);
    api_hal_light_set(LightGreen, g ? 0xFF : 0x00);
    api_hal_light_set(LightBlue, b ? 0xFF : 0x00);
}

int32_t application_blink(void* p) {
    while(1) {
        rgb_set(1, 0, 0);
        delay(500);
        rgb_set(0, 1, 0);
        delay(500);
        rgb_set(0, 0, 1);
        delay(500);
        rgb_set(1, 1, 0);
        delay(500);
        rgb_set(0, 1, 1);
        delay(500);
        rgb_set(1, 0, 1);
        delay(500);
        rgb_set(1, 1, 1);
        delay(500);
        rgb_set(0, 0, 0);
        delay(500);
    }

    return 0;
}