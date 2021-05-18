#include <stdio.h>
#include <furi.h>
#include <api-hal.h>
#include "minunit_vars.h"

int run_minunit();
int run_minunit_test_irda_decoder();

int32_t flipper_test_app(void* p) {
    uint32_t test_result = 0;

    api_hal_light_set(LightRed, 0x00);
    api_hal_light_set(LightGreen, 0x00);
    api_hal_light_set(LightBlue, 0xFF);

    test_result |= run_minunit();
    test_result |= run_minunit_test_irda_decoder();

    /* power_charging_indication_handler() breaks 1 sec light on */
    for(int i = 0; i < 1000; ++i) {
        if(test_result == 0) {
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
        delay(1);
    }

    return 0;
}
