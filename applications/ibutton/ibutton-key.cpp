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

iButtonKey::iButtonKey() {
}
