#pragma once
#include <furi.h>
#include "one_wire_timings.h"

class OneWireMaster {
private:
    const GpioPin* gpio;

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
};