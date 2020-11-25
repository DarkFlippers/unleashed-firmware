#pragma once
#include <stdint.h>
#include "maxim_crc.h"
#include "one_wire_slave_gpio.h"

// TODO fix GPL compability
// currently we use rework of OneWireHub

class OneWireDevice {
public:
    OneWireDevice(
        uint8_t id_1,
        uint8_t id_2,
        uint8_t id_3,
        uint8_t id_4,
        uint8_t id_5,
        uint8_t id_6,
        uint8_t id_7);

    ~OneWireDevice() = default; // TODO: detach if deleted before hub

    // allow only move constructor
    OneWireDevice(OneWireDevice&& one_wire_device) = default;
    OneWireDevice(const OneWireDevice& one_wire_device) = delete;
    OneWireDevice& operator=(OneWireDevice& one_wire_device) = delete;
    OneWireDevice& operator=(const OneWireDevice& one_wire_device) = delete;
    OneWireDevice& operator=(OneWireDevice&& one_wire_device) = delete;

    uint8_t id_storage[8];

    void send_id(OneWireGpioSlave* owner) const;

    virtual void do_work(OneWireGpioSlave* owner) = 0;
};