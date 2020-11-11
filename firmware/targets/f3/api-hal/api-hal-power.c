#include <api-hal-power.h>
#include <bq27220.h>
#include <bq25896.h>

void api_hal_power_init() {
    bq27220_init();
    bq25896_init();
}

uint8_t api_hal_power_get_pct() {
    return bq27220_get_state_of_charge();
}

bool api_hal_power_is_charging() {
    return bq25896_is_charging();
}

void api_hal_power_off() {
    bq25896_poweroff();
}

void api_hal_power_enable_otg() {
    bq25896_enable_otg();
}

void api_hal_power_disable_otg() {
    bq25896_disable_otg();
}
