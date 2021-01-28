#pragma once
#include <stdint.h>
#include "maxim_crc.h"
#include "one_wire_slave.h"

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

    ~OneWireDevice();

    uint8_t id_storage[8];

    void send_id() const;

    OneWireSlave* bus = nullptr;
    void attach(OneWireSlave* _bus);
    void deattach(void);
};