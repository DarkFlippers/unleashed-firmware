#include "one_wire_device.h"

OneWireDevice::OneWireDevice(
    uint8_t id_1,
    uint8_t id_2,
    uint8_t id_3,
    uint8_t id_4,
    uint8_t id_5,
    uint8_t id_6,
    uint8_t id_7) {
    id_storage[0] = id_1;
    id_storage[1] = id_2;
    id_storage[2] = id_3;
    id_storage[3] = id_4;
    id_storage[4] = id_5;
    id_storage[5] = id_6;
    id_storage[6] = id_7;
    id_storage[7] = maxim_crc8(id_storage, 7);
}

OneWireDevice::~OneWireDevice() {
    if(bus != nullptr) {
        bus->deattach();
    }
}

void OneWireDevice::send_id() const {
    if(bus != nullptr) {
        bus->send(id_storage, 8);
    }
}

void OneWireDevice::attach(OneWireSlave* _bus) {
    bus = _bus;
}

void OneWireDevice::deattach(void) {
    bus = nullptr;
}
