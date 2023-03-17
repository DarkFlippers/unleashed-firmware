#include "i2cscanner.h"

void scan_i2c_bus(i2cScanner* i2c_scanner) {
    i2c_scanner->nb_found = 0;
    i2c_scanner->scanned = true;
    // Get the bus
    furi_hal_i2c_acquire(I2C_BUS);
    // scan
    for(uint8_t addr = 0x01; addr <= MAX_I2C_ADDR << 1; addr++) {
        // Check for peripherals
        if(furi_hal_i2c_is_device_ready(I2C_BUS, addr, I2C_TIMEOUT)) {
            // skip even 8-bit addr
            if(addr % 2 != 0) {
                continue;
            }
            // convert addr to 7-bits
            i2c_scanner->addresses[i2c_scanner->nb_found] = addr >> 1;
            i2c_scanner->nb_found++;
        }
    }
    furi_hal_i2c_release(I2C_BUS);
}

i2cScanner* i2c_scanner_alloc() {
    i2cScanner* i2c_scanner = malloc(sizeof(i2cScanner));
    i2c_scanner->nb_found = 0;
    i2c_scanner->menu_index = 0;
    i2c_scanner->scanned = false;
    return i2c_scanner;
}

void i2c_scanner_free(i2cScanner* i2c_scanner) {
    furi_assert(i2c_scanner);
    free(i2c_scanner);
}