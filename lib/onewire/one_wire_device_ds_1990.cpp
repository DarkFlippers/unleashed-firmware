#include "one_wire_device_ds_1990.h"

DS1990::DS1990(
    uint8_t ID1,
    uint8_t ID2,
    uint8_t ID3,
    uint8_t ID4,
    uint8_t ID5,
    uint8_t ID6,
    uint8_t ID7)
    : OneWireDevice(ID1, ID2, ID3, ID4, ID5, ID6, ID7) {
}