#include "gpio_i2c_scanner_control.h"
#include <furi.h>

void gpio_i2c_scanner_run_once(I2CScannerState* i2c_scanner_state) {
    //Reset the number of items for rewriting the array
    i2c_scanner_state->items = 0;
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);

    uint32_t response_timeout_ticks = furi_ms_to_ticks(5.f);

    //Addresses 0 to 7 are reserved and won't be scanned
    for(int i = FIRST_NON_RESERVED_I2C_ADDRESS; i <= HIGHEST_I2C_ADDRESS; i++) {
        if(furi_hal_i2c_is_device_ready(
               &furi_hal_i2c_handle_external,
               i << 1,
               response_timeout_ticks)) { //Bitshift of 1 bit to convert 7-Bit Address into 8-Bit Address
            i2c_scanner_state->responding_address[i2c_scanner_state->items] = i;
            i2c_scanner_state->items++;
        }
    }

    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
}
