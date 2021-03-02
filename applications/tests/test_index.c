#include <stdio.h>
#include <furi.h>
#include <api-hal.h>

int run_minunit();

int32_t flipper_test_app(void* p) {
    api_hal_light_set(LightRed, 0x00);
    api_hal_light_set(LightGreen, 0x00);
    api_hal_light_set(LightBlue, 0xFF);

    uint32_t exitcode = run_minunit();

    if(exitcode == 0) {
        // test passed
        api_hal_light_set(LightRed, 0x00);
        api_hal_light_set(LightGreen, 0xFF);
        api_hal_light_set(LightBlue, 0x00);
    } else {
        // test failed
        api_hal_light_set(LightRed, 0xFF);
        api_hal_light_set(LightGreen, 0x00);
        api_hal_light_set(LightBlue, 0x00);
    }

    return 0;
}