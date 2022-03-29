#include <stdlib.h>
#include "maxim_crc.h"
#include "one_wire_device.h"
#include "one_wire_slave.h"
#include "one_wire_slave_i.h"

struct OneWireDevice {
    uint8_t id_storage[8];
    OneWireSlave* bus;
};

OneWireDevice* onewire_device_alloc(
    uint8_t id_1,
    uint8_t id_2,
    uint8_t id_3,
    uint8_t id_4,
    uint8_t id_5,
    uint8_t id_6,
    uint8_t id_7,
    uint8_t id_8) {
    OneWireDevice* device = malloc(sizeof(OneWireDevice));
    device->id_storage[0] = id_1;
    device->id_storage[1] = id_2;
    device->id_storage[2] = id_3;
    device->id_storage[3] = id_4;
    device->id_storage[4] = id_5;
    device->id_storage[5] = id_6;
    device->id_storage[6] = id_7;
    device->id_storage[7] = id_8;
    device->bus = NULL;

    return device;
}

void onewire_device_free(OneWireDevice* device) {
    if(device->bus != NULL) {
        onewire_slave_detach(device->bus);
    }

    free(device);
}

void onewire_device_send_id(OneWireDevice* device) {
    if(device->bus != NULL) {
        onewire_slave_send(device->bus, device->id_storage, 8);
    }
}

void onewire_device_attach(OneWireDevice* device, OneWireSlave* bus) {
    device->bus = bus;
}

void onewire_device_detach(OneWireDevice* device) {
    device->bus = NULL;
}

uint8_t* onewire_device_get_id_p(OneWireDevice* device) {
    return device->id_storage;
}
