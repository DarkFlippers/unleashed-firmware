#pragma once
#include "flipper.h"
#include "flipper_v2.h"
#include "one_wire_timings.h"

class OneWireGpioSlave {
private:
    GpioPin* gpio;

public:
    OneWireGpioSlave(GpioPin* one_wire_gpio);
    ~OneWireGpioSlave();
    void start(void);
    void stop(void);
    bool emulate(uint8_t* buffer, uint8_t length);

    bool check_reset(void);
    bool show_presence(void);
    bool receive_and_process_cmd(void);
    bool receive(uint8_t* data, const uint8_t data_length);
    bool receiveBit(void);

    bool overdrive_mode = false;

    OneWiteTimeType wait_while_gpio(volatile OneWiteTimeType retries, const bool pin_value);
};

OneWireGpioSlave::OneWireGpioSlave(GpioPin* one_wire_gpio) {
    gpio = one_wire_gpio;
}

OneWireGpioSlave::~OneWireGpioSlave() {
    stop();
}

void OneWireGpioSlave::start(void) {
    gpio_init(gpio, GpioModeOutputOpenDrain);
}

void OneWireGpioSlave::stop(void) {
    gpio_init(gpio, GpioModeAnalog);
}

bool OneWireGpioSlave::emulate(uint8_t* buffer, uint8_t length) {
    if(!check_reset()) {
        printf("reset error\n");
        return false;
    }

    if(!show_presence()) {
        printf("presence error\n");
        return false;
    }

    if(!receive_and_process_cmd()) {
        printf("receive_and_process_cmd error\n");
        return false;
    }

    printf("ok\n");
    return true;
}

OneWiteTimeType OneWireGpioSlave::wait_while_gpio(OneWiteTimeType time, const bool pin_value) {
    uint32_t start = DWT->CYCCNT;
    uint32_t time_ticks = time * (SystemCoreClock / 1000000.0f);

    while(((DWT->CYCCNT - start) < time_ticks)) {
        if(gpio_read(gpio) != pin_value) {
            uint32_t time = (DWT->CYCCNT - start);
            time /= (SystemCoreClock / 1000000.0f);
            return time;
        }
    }

    return 0;
}

bool OneWireGpioSlave::check_reset(void) {
    while(gpio_read(gpio) == true) {
    }

    /*if(wait_while_gpio(OneWireEmulateTiming::RESET_TIMEOUT * 20, true) == 0) {
        printf("RESET_TIMEOUT\n");
        return false;
    }*/

    const OneWiteTimeType time_remaining =
        wait_while_gpio(OneWireEmulateTiming::RESET_MAX[0], false);

    if(time_remaining == 0) {
        return false;
    }

    if(overdrive_mode && ((OneWireEmulateTiming::RESET_MAX[0] -
                           OneWireEmulateTiming::RESET_MIN[0]) <= time_remaining)) {
        // normal reset detected
        overdrive_mode = false;
    };

    bool result = (time_remaining <= OneWireEmulateTiming::RESET_MAX[0]) &&
                  time_remaining >= OneWireEmulateTiming::RESET_MIN[overdrive_mode];

    return result;
}

bool OneWireGpioSlave::show_presence(void) {
    wait_while_gpio(OneWireEmulateTiming::PRESENCE_TIMEOUT, true);
    gpio_write(gpio, false);
    delay_us(OneWireEmulateTiming::PRESENCE_MIN[overdrive_mode]);
    gpio_write(gpio, true);
    /*OneWiteTimeType wait_time = OneWireEmulateTiming::PRESENCE_MAX[overdrive_mode] -
                                OneWireEmulateTiming::PRESENCE_MIN[overdrive_mode];
    if(wait_while_gpio(wait_time, false) == 0) {
        return false;
    }*/

    return true;
}

bool OneWireGpioSlave::receive_and_process_cmd(void) {
    uint8_t cmd;
    receive(&cmd, 1);
    printf("cmd %x\n", cmd);
    return false;
}

bool OneWireGpioSlave::receiveBit(void) {
    // wait while bus is HIGH
    OneWiteTimeType time = OneWireEmulateTiming::SLOT_MAX[overdrive_mode];
    time = wait_while_gpio(time, true);
    if (time == 0)
    {
        printf("RESET_IN_PROGRESS\n");
        return false;
    }
    /*while ((DIRECT_READ(pin_baseReg, pin_bitMask) == 0) && (--retries != 0));
    if (retries == 0)
    {
        _error = Error::RESET_IN_PROGRESS;
        return false;
    }*/

    // wait while bus is LOW
    time = OneWireEmulateTiming::MSG_HIGH_TIMEOUT;
    time = wait_while_gpio(time, false);
    if (time == 0)
    {
        printf("TIMEOUT_HIGH\n");
        return false;
    }
    /*while ((DIRECT_READ(pin_baseReg, pin_bitMask) != 0) && (--retries != 0));
    if (retries == 0)
    {
        _error = Error::AWAIT_TIMESLOT_TIMEOUT_HIGH;
        return false;
    }*/

    // wait a specific time to do a read (data is valid by then), // first difference to inner-loop of write()
    time = OneWireEmulateTiming::READ_MIN[overdrive_mode];
    time = wait_while_gpio(time, true);
    //while ((DIRECT_READ(pin_baseReg, pin_bitMask) == 0) && (--retries != 0));

    return (time > 0);
}

bool OneWireGpioSlave::receive(uint8_t* data, const uint8_t data_length) {
    uint8_t bytes_received = 0;
    for(; bytes_received < data_length; ++bytes_received) {
        uint8_t value = 0;

        for(uint8_t bitMask = 0x01; bitMask != 0; bitMask <<= 1) {
            if(receiveBit()) value |= bitMask;
        }

        data[bytes_received] = value;
    }
    return (bytes_received != data_length);
}