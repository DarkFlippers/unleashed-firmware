#pragma once
#include "one_wire_device.h"

// TODO fix GPL compability
// currently we use rework of OneWireHub

class DS1990 : public OneWireDevice {
public:
    static constexpr uint8_t family_code{0x01};

    DS1990(
        uint8_t ID1,
        uint8_t ID2,
        uint8_t ID3,
        uint8_t ID4,
        uint8_t ID5,
        uint8_t ID6,
        uint8_t ID7);

    void do_work(OneWireGpioSlave* owner) final;
};