#pragma once
#include <furi.h>
#include <furi-hal.h>
#include "one_wire_timings.h"

class OneWireDevice;
typedef void (*OneWireSlaveResultCallback)(bool success, void* ctx);

class OneWireSlave {
private:
    enum class OneWireSlaveError : uint8_t {
        NO_ERROR = 0,
        READ_TIMESLOT_TIMEOUT,
        WRITE_TIMESLOT_TIMEOUT,
        WAIT_RESET_TIMEOUT,
        VERY_LONG_RESET,
        VERY_SHORT_RESET,
        PRESENCE_LOW_ON_LINE,
        READ_TIMESLOT_TIMEOUT_LOW,
        AWAIT_TIMESLOT_TIMEOUT_HIGH,
        PRESENCE_HIGH_ON_LINE,
        INCORRECT_ONEWIRE_CMD,
        INCORRECT_SLAVE_USAGE,
        TRIED_INCORRECT_WRITE,
        FIRST_TIMESLOT_TIMEOUT,
        FIRST_BIT_OF_BYTE_TIMEOUT,
        RESET_IN_PROGRESS
    };

    const GpioPin* one_wire_pin_record;

    // exti callback and its pointer
    void exti_callback(void* _ctx);
    void (*exti_cb)(void* _ctx);

    uint32_t __instructions_per_us;

    OneWireSlaveError error;
    OneWireDevice* device = nullptr;

    bool bus_start(void);

    void pin_set_float(void);
    void pin_set_low(void);
    void pin_init_interrupt_in_isr_ctx(void);
    void pin_init_opendrain_in_isr_ctx(void);

    OneWiteTimeType wait_while_gpio_is(OneWiteTimeType time, const bool pin_value);

    bool show_presence(void);
    bool receive_and_process_cmd(void);

    bool receive_bit(void);
    bool send_bit(bool value);

    void cmd_search_rom(void);

    OneWireSlaveResultCallback result_cb = nullptr;
    void* result_cb_ctx = nullptr;

public:
    void start(void);
    void stop(void);

    bool send(const uint8_t* address, const uint8_t data_length);
    bool receive(uint8_t* data, const uint8_t data_length = 1);

    OneWireSlave(const GpioPin* pin);
    ~OneWireSlave();

    void attach(OneWireDevice* device);
    void deattach(void);

    void set_result_callback(OneWireSlaveResultCallback result_cb, void* ctx);
};