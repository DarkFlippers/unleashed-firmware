#include "one_wire_slave_gpio.h"
#include "one_wire_device.h"
#include "one_wire_device_ds_1990.h"

// TODO fix GPL compability
// currently we use rework of OneWireHub

static uint32_t __instructions_per_us = 0;

OneWireGpioSlave::OneWireGpioSlave(const GpioPin* one_wire_gpio) {
    gpio = one_wire_gpio;
    error = OneWireGpioSlaveError::NO_ERROR;
    devices_count = 0;
    device_selected = nullptr;

    for(uint8_t i = 0; i < ONE_WIRE_MAX_DEVICES; ++i) {
        devices[i] = nullptr;
    }

    __instructions_per_us = (SystemCoreClock / 1000000.0f);
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

bool OneWireGpioSlave::emulate() {
    bool anything_emulated = false;
    error = OneWireGpioSlaveError::NO_ERROR;

    while(1) {
        if(devices_count == 0) return false;

        if(!check_reset()) {
            return anything_emulated;
        } else {
        }

        // OK, we receive reset
        osKernelLock();

        if(!show_presence()) {
            return anything_emulated;
        } else {
            anything_emulated = true;
        }

        // and we succefully show our presence on bus
        __disable_irq();

        // TODO think about return condition
        if(!receive_and_process_cmd()) {
            __enable_irq();
            osKernelUnlock();
        } else {
            __enable_irq();
            osKernelUnlock();
        }
    }
}

OneWiteTimeType OneWireGpioSlave::wait_while_gpio_is(OneWiteTimeType time, const bool pin_value) {
    uint32_t start = DWT->CYCCNT;
    uint32_t time_ticks = time * __instructions_per_us;
    uint32_t time_captured;

    do {
        time_captured = DWT->CYCCNT;
        if(gpio_read(gpio) != pin_value) {
            OneWiteTimeType remaining_time = time_ticks - (time_captured - start);
            remaining_time /= __instructions_per_us;
            return remaining_time;
        }
    } while((time_captured - start) < time_ticks);

    return 0;
}

void OneWireGpioSlave::pin_set_float() {
    gpio_write(gpio, true);
}

void OneWireGpioSlave::pin_set_low() {
    gpio_write(gpio, false);
}

const char* OneWireGpioSlave::decode_error() {
    const char* error_text[16] = {
        "NO_ERROR",
        "READ_TIMESLOT_TIMEOUT",
        "WRITE_TIMESLOT_TIMEOUT",
        "WAIT_RESET_TIMEOUT",
        "VERY_LONG_RESET",
        "VERY_SHORT_RESET",
        "PRESENCE_LOW_ON_LINE",
        "READ_TIMESLOT_TIMEOUT_LOW",
        "AWAIT_TIMESLOT_TIMEOUT_HIGH",
        "PRESENCE_HIGH_ON_LINE",
        "INCORRECT_ONEWIRE_CMD",
        "INCORRECT_SLAVE_USAGE",
        "TRIED_INCORRECT_WRITE",
        "FIRST_TIMESLOT_TIMEOUT",
        "FIRST_BIT_OF_BYTE_TIMEOUT",
        "RESET_IN_PROGRESS"};

    return error_text[static_cast<uint8_t>(error)];
}

uint8_t OneWireGpioSlave::attach(OneWireDevice& device) {
    if(devices_count >= ONE_WIRE_MAX_DEVICES) return 255; // hub is full

    uint8_t position = 255;
    for(uint8_t i = 0; i < ONE_WIRE_MAX_DEVICES; ++i) {
        if(devices[i] == &device) {
            return i;
        }
        if((position > ONE_WIRE_MAX_DEVICES) && (devices[i] == nullptr)) {
            position = i;
        }
    }

    if(position == 255) return 255;

    devices[position] = &device;
    devices_count++;
    build_id_tree();
    return position;
}

bool OneWireGpioSlave::detach(const OneWireDevice& device) {
    uint8_t position = 255;

    for(uint8_t i = 0; i < ONE_WIRE_MAX_DEVICES; ++i) {
        if(devices[i] == &device) {
            position = i;
            break;
        }
    }

    if(position != 255) return detach(position);

    return false;
}

bool OneWireGpioSlave::detach(uint8_t device_number) {
    if(devices[device_number] == nullptr) return false;
    if(devices_count == 0) return false;
    if(device_number >= ONE_WIRE_MAX_DEVICES) return false;

    devices[device_number] = nullptr;
    devices_count--;
    build_id_tree();

    return true;
}

uint8_t OneWireGpioSlave::get_next_device_index(const uint8_t index_start) const {
    for(uint8_t i = index_start; i < ONE_WIRE_MAX_DEVICES; ++i) {
        if(devices[i] != nullptr) return i;
    }
    return 0;
}

uint8_t OneWireGpioSlave::build_id_tree(void) {
    uint32_t device_mask = 0;
    uint32_t bit_mask = 0x01;

    // build mask
    for(uint8_t i = 0; i < ONE_WIRE_MAX_DEVICES; ++i) {
        if(devices[i] != nullptr) device_mask |= bit_mask;
        bit_mask <<= 1;
    }

    for(uint8_t i = 0; i < ONE_WIRE_MAX_DEVICES; ++i) {
        id_tree[i].id_position = 255;
    }

    // begin with root-element
    build_id_tree(0, device_mask); // goto branch

    return 0;
}

uint8_t OneWireGpioSlave::build_id_tree(uint8_t id_bit_position, uint32_t device_mask) {
    if(device_mask == 0) return (255);

    while(id_bit_position < 64) {
        uint32_t mask_pos{0};
        uint32_t mask_neg{0};
        const uint8_t pos_byte{static_cast<uint8_t>(id_bit_position >> 3)};
        const uint8_t mask_bit{static_cast<uint8_t>(1 << (id_bit_position & 7))};
        uint32_t mask_id{1};

        // searchid_tree through all active slaves
        for(uint8_t id = 0; id < ONE_WIRE_MAX_DEVICES; ++id) {
            if((device_mask & mask_id) != 0) {
                // if slave is in mask differentiate the bitValue
                if((devices[id]->id_storage[pos_byte] & mask_bit) != 0)
                    mask_pos |= mask_id;
                else
                    mask_neg |= mask_id;
            }
            mask_id <<= 1;
        }

        if((mask_neg != 0) && (mask_pos != 0)) {
            // there was found a junction
            const uint8_t active_element = get_first_id_tree_el_position();

            id_tree[active_element].id_position = id_bit_position;
            id_tree[active_element].device_selected = get_first_bit_set_position(device_mask);
            id_bit_position++;
            id_tree[active_element].got_one = build_id_tree(id_bit_position, mask_pos);
            id_tree[active_element].got_zero = build_id_tree(id_bit_position, mask_neg);
            return active_element;
        }

        id_bit_position++;
    }

    // gone through the address, store this result
    uint8_t active_element = get_first_id_tree_el_position();

    id_tree[active_element].id_position = 128;
    id_tree[active_element].device_selected = get_first_bit_set_position(device_mask);
    id_tree[active_element].got_one = 255;
    id_tree[active_element].got_zero = 255;

    return active_element;
}

uint8_t OneWireGpioSlave::get_first_bit_set_position(uint32_t mask) const {
    uint32_t _mask = mask;
    for(uint8_t i = 0; i < ONE_WIRE_MAX_DEVICES; ++i) {
        if((_mask & 1) != 0) return i;
        _mask >>= 1;
    }
    return 0;
}

uint8_t OneWireGpioSlave::get_first_id_tree_el_position(void) const {
    for(uint8_t i = 0; i < ONE_WIRE_MAX_DEVICES; ++i) {
        if(id_tree[i].id_position == 255) return i;
    }
    return 0;
}

void OneWireGpioSlave::cmd_search_rom(void) {
    uint8_t id_bit_position = 0;
    uint8_t trigger_position = 0;
    uint8_t active_slave = id_tree[trigger_position].device_selected;
    uint8_t trigger_bit = id_tree[trigger_position].id_position;

    while(id_bit_position < 64) {
        // if junction is reached, act different
        if(id_bit_position == trigger_bit) {
            if(!send_bit(false)) return;
            if(!send_bit(false)) return;

            const bool bit_recv = receive_bit();
            if(error != OneWireGpioSlaveError::NO_ERROR) return;

            // switch to next junction
            trigger_position = bit_recv ? id_tree[trigger_position].got_one :
                                          id_tree[trigger_position].got_zero;

            active_slave = id_tree[trigger_position].device_selected;

            trigger_bit = (trigger_position == 255) ? uint8_t(255) :
                                                      id_tree[trigger_position].id_position;
        } else {
            const uint8_t pos_byte = (id_bit_position >> 3);
            const uint8_t mask_bit = (static_cast<uint8_t>(1) << (id_bit_position & (7)));
            bool bit_send;

            if((devices[active_slave]->id_storage[pos_byte] & mask_bit) != 0) {
                bit_send = true;
                if(!send_bit(true)) return;
                if(!send_bit(false)) return;
            } else {
                bit_send = false;
                if(!send_bit(false)) return;
                if(!send_bit(true)) return;
            }

            const bool bit_recv = receive_bit();
            if(error != OneWireGpioSlaveError::NO_ERROR) return;

            if(bit_send != bit_recv) return;
        }
        id_bit_position++;
    }

    device_selected = devices[active_slave];
}

bool OneWireGpioSlave::check_reset(void) {
    pin_set_float();

    if(error == OneWireGpioSlaveError::RESET_IN_PROGRESS) {
        error = OneWireGpioSlaveError::NO_ERROR;

        if(wait_while_gpio_is(
               OWET::RESET_MIN[overdrive_mode] - OWET::SLOT_MAX[overdrive_mode] -
                   OWET::READ_MAX[overdrive_mode],
               false) == 0) {
            // we want to show_presence on high, so wait for it
            const OneWiteTimeType time_remaining = wait_while_gpio_is(OWET::RESET_MAX[0], false);

            if(overdrive_mode &&
               ((OWET::RESET_MAX[0] - OWET::RESET_MIN[overdrive_mode]) > time_remaining)) {
                overdrive_mode = false;
            };

            return true;
        }
    }

    // if line is low, then just leave
    if(gpio_read(gpio) == 0) {
        return false;
    }

    // wait while gpio is high
    if(wait_while_gpio_is(OWET::RESET_TIMEOUT, true) == 0) {
        return false;
    }

    // store low time
    OneWiteTimeType time_remaining = wait_while_gpio_is(OWET::RESET_MAX[0], false);

    // low time more than RESET_MAX time
    if(time_remaining == 0) {
        error = OneWireGpioSlaveError::VERY_LONG_RESET;
        return false;
    }

    // get real reset time
    time_remaining = OWET::RESET_MAX[0] - time_remaining;

    // if time, while bus was low, fit in standart reset timings
    if(overdrive_mode && ((OWET::RESET_MAX[0] - OWET::RESET_MIN[0]) <= time_remaining)) {
        // normal reset detected
        overdrive_mode = false;
    };

    bool result = (time_remaining <= OWET::RESET_MAX[0]) &&
                  time_remaining >= OWET::RESET_MIN[overdrive_mode];

    return result;
}

bool OneWireGpioSlave::show_presence(void) {
    // wait while master delay presence check
    wait_while_gpio_is(OWET::PRESENCE_TIMEOUT, true);

    // show presence
    pin_set_low();
    delay_us(OWET::PRESENCE_MIN[overdrive_mode]);
    pin_set_float();

    // somebody also can show presence
    const OneWiteTimeType wait_low_time =
        OWET::PRESENCE_MAX[overdrive_mode] - OWET::PRESENCE_MIN[overdrive_mode];

    // so we will wait
    if(wait_while_gpio_is(wait_low_time, false) == 0) {
        error = OneWireGpioSlaveError::PRESENCE_LOW_ON_LINE;
        return false;
    }

    return true;
}

bool OneWireGpioSlave::receive_and_process_cmd(void) {
    receive(&cmd);

    if(error == OneWireGpioSlaveError::RESET_IN_PROGRESS) return true;
    if(error != OneWireGpioSlaveError::NO_ERROR) return false;

    switch(cmd) {
    case 0xF0:
        // SEARCH ROM
        device_selected = nullptr;
        cmd_search_rom();

        // trigger reinit
        return true;

    case 0x33:
        // READ ROM

        // work only when one slave on the bus
        if((device_selected == nullptr) && (devices_count == 1)) {
            device_selected = devices[get_next_device_index()];
        }
        if(device_selected != nullptr) {
            device_selected->send_id(this);
        }
        return false;

    default: // Unknown command
        error = OneWireGpioSlaveError::INCORRECT_ONEWIRE_CMD;
        //error_cmd = cmd;
    }

    if(error == OneWireGpioSlaveError::RESET_IN_PROGRESS) return true;
    return (error == OneWireGpioSlaveError::NO_ERROR);
}

bool OneWireGpioSlave::receive_bit(void) {
    // wait while bus is low
    OneWiteTimeType time = OWET::SLOT_MAX[overdrive_mode];
    time = wait_while_gpio_is(time, false);
    if(time == 0) {
        error = OneWireGpioSlaveError::RESET_IN_PROGRESS;
        return false;
    }

    // wait while bus is high
    time = OWET::MSG_HIGH_TIMEOUT;
    time = wait_while_gpio_is(time, true);
    if(time == 0) {
        error = OneWireGpioSlaveError::AWAIT_TIMESLOT_TIMEOUT_HIGH;
        error_place = 1;
        return false;
    }

    // wait a time of zero
    time = OWET::READ_MIN[overdrive_mode];
    time = wait_while_gpio_is(time, false);

    return (time > 0);
}

bool OneWireGpioSlave::send_bit(bool value) {
    const bool write_zero = !value;

    // wait while bus is low
    OneWiteTimeType time = OWET::SLOT_MAX[overdrive_mode];
    time = wait_while_gpio_is(time, false);
    if(time == 0) {
        error = OneWireGpioSlaveError::RESET_IN_PROGRESS;
        return false;
    }

    // wait while bus is high
    time = OWET::MSG_HIGH_TIMEOUT;
    time = wait_while_gpio_is(time, true);
    if(time == 0) {
        error = OneWireGpioSlaveError::AWAIT_TIMESLOT_TIMEOUT_HIGH;
        error_place = 2;
        return false;
    }

    // choose write time
    if(write_zero) {
        pin_set_low();
        time = OWET::WRITE_ZERO[overdrive_mode];
    } else {
        time = OWET::READ_MAX[overdrive_mode];
    }

    // hold line for ZERO or ONE time
    delay_us(time);
    pin_set_float();

    return true;
}

bool OneWireGpioSlave::send(const uint8_t* address, const uint8_t data_length) {
    uint8_t bytes_sent = 0;

    pin_set_float();

    // bytes loop
    for(; bytes_sent < data_length; ++bytes_sent) {
        const uint8_t data_byte = address[bytes_sent];

        // bit loop
        for(uint8_t bit_mask = 0x01; bit_mask != 0; bit_mask <<= 1) {
            if(!send_bit(static_cast<bool>(bit_mask & data_byte))) {
                // if we cannot send first bit
                if((bit_mask == 0x01) &&
                   (error == OneWireGpioSlaveError::AWAIT_TIMESLOT_TIMEOUT_HIGH))
                    error = OneWireGpioSlaveError::FIRST_BIT_OF_BYTE_TIMEOUT;
                return false;
            }
        }
    }
    return true;
}

bool OneWireGpioSlave::receive(uint8_t* data, const uint8_t data_length) {
    uint8_t bytes_received = 0;

    pin_set_float();

    for(; bytes_received < data_length; ++bytes_received) {
        uint8_t value = 0;

        for(uint8_t bit_mask = 0x01; bit_mask != 0; bit_mask <<= 1) {
            if(receive_bit()) value |= bit_mask;
        }

        data[bytes_received] = value;
    }
    return (bytes_received != data_length);
}