#pragma once

#include <furi.h>
#include <furi_hal.h>

class CyfralTiming {
public:
    constexpr static const uint8_t ZERO_HIGH = 50;
    constexpr static const uint8_t ZERO_LOW = 70;
    constexpr static const uint8_t ONE_HIGH = 100;
    constexpr static const uint8_t ONE_LOW = 70;
};

class CyfralEmulator {
private:
    void send_nibble(uint8_t nibble);
    void send_byte(uint8_t data);
    inline void send_bit(bool bit);
    const GpioPin* emulate_pin_record;

public:
    CyfralEmulator(const GpioPin* emulate_pin);
    ~CyfralEmulator();
    void send(uint8_t* data, uint8_t count = 1, uint8_t repeat = 1);
    void start(void);
    void stop(void);
};

// 7 = 0 1 1 1
// B = 1 0 1 1
// D = 1 1 0 1
// E = 1 1 1 0

void CyfralEmulator::send_nibble(uint8_t nibble) {
    for(uint8_t i = 0; i < 4; i++) {
        bool bit = nibble & (0b1000 >> i);
        send_bit(bit);
    }
}

void CyfralEmulator::send_byte(uint8_t data) {
    for(uint8_t i = 0; i < 8; i++) {
        bool bit = data & (0b10000000 >> i);
        send_bit(bit);
    }
}

void CyfralEmulator::send_bit(bool bit) {
    if(!bit) {
        hal_gpio_write(&ibutton_gpio, false);
        delay_us(CyfralTiming::ZERO_LOW);
        hal_gpio_write(&ibutton_gpio, true);
        delay_us(CyfralTiming::ZERO_HIGH);
        hal_gpio_write(&ibutton_gpio, false);
        delay_us(CyfralTiming::ZERO_LOW);
    } else {
        hal_gpio_write(&ibutton_gpio, true);
        delay_us(CyfralTiming::ONE_HIGH);
        hal_gpio_write(&ibutton_gpio, false);
        delay_us(CyfralTiming::ONE_LOW);
    }
}

CyfralEmulator::CyfralEmulator(const GpioPin* emulate_pin) {
    emulate_pin_record = emulate_pin;
}

CyfralEmulator::~CyfralEmulator() {
}

void CyfralEmulator::send(uint8_t* data, uint8_t count, uint8_t repeat) {
    osKernelLock();
    __disable_irq();

    for(uint8_t i = 0; i < repeat; i++) {
        // start sequence
        send_nibble(0x01);

        // send data
        for(uint8_t i = 0; i < count; i++) {
            send_byte(data[i]);
        }
    }

    __enable_irq();
    osKernelUnlock();
}

void CyfralEmulator::start(void) {
    hal_gpio_init(emulate_pin_record, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
    hal_gpio_write(emulate_pin_record, false);
}

void CyfralEmulator::stop(void) {
    hal_gpio_init(emulate_pin_record, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
}