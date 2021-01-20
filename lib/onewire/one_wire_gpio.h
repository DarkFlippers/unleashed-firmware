#pragma once
#include <furi.h>
#include "one_wire_timings.h"

class OneWireGpio {
private:
    const GpioPin* gpio;

public:
    OneWireGpio(const GpioPin* one_wire_gpio);
    ~OneWireGpio();
    bool reset(void);
    bool read_bit(void);
    uint8_t read(void);
    void read_bytes(uint8_t* buf, uint16_t count);
    void write_bit(bool value);
    void write(uint8_t value);
    void start(void);
    void stop(void);
};

OneWireGpio::OneWireGpio(const GpioPin* one_wire_gpio) {
    gpio = one_wire_gpio;
}

OneWireGpio::~OneWireGpio() {
    stop();
}

void OneWireGpio::start(void) {
    gpio_init(gpio, GpioModeOutputOpenDrain);
}

void OneWireGpio::stop(void) {
    gpio_init(gpio, GpioModeAnalog);
}

bool OneWireGpio::reset(void) {
    uint8_t r;
    uint8_t retries = 125;

    // wait until the gpio is high
    gpio_write(gpio, true);
    do {
        if(--retries == 0) return 0;
        delay_us(2);
    } while(!gpio_read(gpio));

    // pre delay
    delay_us(OneWireTiming::RESET_DELAY_PRE);

    // drive low
    gpio_write(gpio, false);
    delay_us(OneWireTiming::RESET_DRIVE);

    // release
    gpio_write(gpio, true);
    delay_us(OneWireTiming::RESET_RELEASE);

    // read and post delay
    r = !gpio_read(gpio);
    delay_us(OneWireTiming::RESET_DELAY_POST);

    return r;
}

bool OneWireGpio::read_bit(void) {
    bool result;

    // drive low
    gpio_write(gpio, false);
    delay_us(OneWireTiming::READ_DRIVE);

    // release
    gpio_write(gpio, true);
    delay_us(OneWireTiming::READ_RELEASE);

    // read and post delay
    result = gpio_read(gpio);
    delay_us(OneWireTiming::READ_DELAY_POST);

    return result;
}

void OneWireGpio::write_bit(bool value) {
    if(value) {
        // drive low
        gpio_write(gpio, false);
        delay_us(OneWireTiming::WRITE_1_DRIVE);

        // release
        gpio_write(gpio, true);
        delay_us(OneWireTiming::WRITE_1_RELEASE);
    } else {
        // drive low
        gpio_write(gpio, false);
        delay_us(OneWireTiming::WRITE_0_DRIVE);

        // release
        gpio_write(gpio, true);
        delay_us(OneWireTiming::WRITE_0_RELEASE);
    }
}

uint8_t OneWireGpio::read(void) {
    uint8_t result = 0;

    for(uint8_t bitMask = 0x01; bitMask; bitMask <<= 1) {
        if(read_bit()) {
            result |= bitMask;
        }
    }

    return result;
}

void OneWireGpio::read_bytes(uint8_t* buffer, uint16_t count) {
    for(uint16_t i = 0; i < count; i++) {
        buffer[i] = read();
    }
}

void OneWireGpio::write(uint8_t value) {
    uint8_t bitMask;

    for(bitMask = 0x01; bitMask; bitMask <<= 1) {
        write_bit((bitMask & value) ? 1 : 0);
    }
}
