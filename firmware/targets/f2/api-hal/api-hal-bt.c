#include <api-hal-bt.h>

void api_hal_bt_init() {
}

void api_hal_bt_dump_state(string_t buffer) {
    string_cat_printf(buffer, "BLE unsupported");
}

bool api_hal_bt_is_alive() {
    return false;
}