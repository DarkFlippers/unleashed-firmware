#pragma once
#include <furi.h>
#include <furi_hal.h>
#include "one_wire_timings.h"

class OneWireMaster {
private:
    const GpioPin* gpio;

    // global search state
    unsigned char saved_rom[8];
    uint8_t last_discrepancy;
    uint8_t last_family_discrepancy;
    bool last_device_flag;

public:
    OneWireMaster(const GpioPin* one_wire_gpio);
    ~OneWireMaster();
    bool reset(void);
    bool read_bit(void);
    uint8_t read(void);
    void read_bytes(uint8_t* buf, uint16_t count);
    void write_bit(bool value);
    void write(uint8_t value);
    void skip(void);
    void start(void);
    void stop(void);

    void reset_search();
    void target_search(uint8_t family_code);
    uint8_t search(uint8_t* newAddr, bool search_mode = true);
};