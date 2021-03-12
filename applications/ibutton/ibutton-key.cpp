#include "ibutton-key.h"
#include <furi.h>

uint8_t iButtonKey::get_size() {
    return IBUTTON_KEY_SIZE;
}

void iButtonKey::set_data(uint8_t* _data, uint8_t _data_count) {
    furi_check(_data_count > 0);
    furi_check(_data_count <= get_size());

    memset(data, 0, get_size());
    memcpy(data, _data, _data_count);
}

uint8_t* iButtonKey::get_data() {
    return data;
}

uint8_t iButtonKey::get_type_data_size() {
    uint8_t size = 0;

    switch(type) {
    case iButtonKeyType::KeyCyfral:
        size = 2;
        break;
    case iButtonKeyType::KeyMetakom:
        size = 4;
        break;
    case iButtonKeyType::KeyDallas:
        size = 8;
        break;
    }

    return size;
}

void iButtonKey::set_name(const char* _name) {
    name = _name;
}

const char* iButtonKey::get_name() {
    return name;
}

void iButtonKey::set_type(iButtonKeyType _key_type) {
    type = _key_type;
}

iButtonKeyType iButtonKey::get_key_type() {
    return type;
}

iButtonKey::iButtonKey(
    iButtonKeyType _type,
    const char* _name,
    uint8_t d0,
    uint8_t d1,
    uint8_t d2,
    uint8_t d3,
    uint8_t d4,
    uint8_t d5,
    uint8_t d6,
    uint8_t d7) {
    type = _type;
    name = _name;
    data[0] = d0;
    data[1] = d1;
    data[2] = d2;
    data[3] = d3;
    data[4] = d4;
    data[5] = d5;
    data[6] = d6;
    data[7] = d7;
}

iButtonKey::iButtonKey() {
}
