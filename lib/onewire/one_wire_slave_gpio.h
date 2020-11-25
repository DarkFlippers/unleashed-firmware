#pragma once
#include "flipper.h"
#include "flipper_v2.h"
#include "one_wire_timings.h"

// TODO fix GPL compability
// currently we use rework of OneWireHub

#define ONE_WIRE_MAX_DEVICES 1
#define ONE_WIRE_TREE_SIZE ((2 * ONE_WIRE_MAX_DEVICES) - 1)

#define OWET OneWireEmulateTiming

class OneWireDevice;

enum class OneWireGpioSlaveError : uint8_t {
    NO_ERROR = 0,
    READ_TIMESLOT_TIMEOUT = 1,
    WRITE_TIMESLOT_TIMEOUT = 2,
    WAIT_RESET_TIMEOUT = 3,
    VERY_LONG_RESET = 4,
    VERY_SHORT_RESET = 5,
    PRESENCE_LOW_ON_LINE = 6,
    READ_TIMESLOT_TIMEOUT_LOW = 7,
    AWAIT_TIMESLOT_TIMEOUT_HIGH = 8,
    PRESENCE_HIGH_ON_LINE = 9,
    INCORRECT_ONEWIRE_CMD = 10,
    INCORRECT_SLAVE_USAGE = 11,
    TRIED_INCORRECT_WRITE = 12,
    FIRST_TIMESLOT_TIMEOUT = 13,
    FIRST_BIT_OF_BYTE_TIMEOUT = 14,
    RESET_IN_PROGRESS = 15
};

class OneWireGpioSlave {
private:
    const GpioPin* gpio;
    bool overdrive_mode = false;
    uint8_t cmd;
    OneWireGpioSlaveError error;
    uint8_t error_place;

    uint8_t devices_count;
    OneWireDevice* devices[ONE_WIRE_MAX_DEVICES];
    OneWireDevice* device_selected;

    struct IDTree {
        uint8_t device_selected; // for which slave is this jump-command relevant
        uint8_t id_position; // where does the algorithm has to look for a junction
        uint8_t got_zero; // if 0 switch to which tree branch
        uint8_t got_one; // if 1 switch to which tree branch
    } id_tree[ONE_WIRE_TREE_SIZE];

public:
    OneWireGpioSlave(const GpioPin* one_wire_gpio);
    ~OneWireGpioSlave();

    void start(void);
    void stop(void);
    bool emulate();
    bool check_reset(void);
    bool show_presence(void);
    bool receive_and_process_cmd(void);
    bool receive(uint8_t* data, const uint8_t data_length = 1);
    bool receive_bit(void);
    bool send_bit(bool value);
    bool send(const uint8_t* address, const uint8_t data_length = 1);

    OneWiteTimeType wait_while_gpio_is(volatile OneWiteTimeType retries, const bool pin_value);

    // set pin state
    inline void pin_set_float();
    inline void pin_set_low();

    // get error text
    const char* decode_error();

    // devices managment
    uint8_t attach(OneWireDevice& device);
    bool detach(const OneWireDevice& device);
    bool detach(uint8_t device_number);
    uint8_t get_next_device_index(const uint8_t index_start = 0) const;

    // id tree managment
    uint8_t build_id_tree(void);
    uint8_t build_id_tree(uint8_t id_bit_position, uint32_t device_mask);

    uint8_t get_first_bit_set_position(uint32_t mask) const;
    uint8_t get_first_id_tree_el_position(void) const;

    // commands
    void cmd_search_rom(void);
};