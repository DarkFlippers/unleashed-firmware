#include "ibutton-key.h"
#include <furi.h>

uint8_t iButtonKey::get_size() {
    return IBUTTON_KEY_DATA_SIZE;
}

void iButtonKey::set_data(uint8_t* _data, uint8_t _data_count) {
    furi_check(_data_count > 0);
    furi_check(_data_count <= get_size());

    memset(data, 0, get_size());
    memcpy(data, _data, _data_count);
}

void iButtonKey::clear_data() {
    memset(data, 0, get_size());
}

uint8_t* iButtonKey::get_data() {
    return data;
}

uint8_t iButtonKey::get_type_data_size() {
    return get_type_data_size_by_type(type);
}

void iButtonKey::set_name(const char* _name) {
    strlcpy(name, _name, IBUTTON_KEY_NAME_SIZE);
}

char* iButtonKey::get_name() {
    return name;
}

void iButtonKey::set_type(iButtonKeyType _key_type) {
    type = _key_type;
}

iButtonKeyType iButtonKey::get_key_type() {
    return type;
}

const char* iButtonKey::get_key_type_string_by_type(iButtonKeyType key_type) {
    switch(key_type) {
    case iButtonKeyType::KeyCyfral:
        return "Cyfral";
        break;
    case iButtonKeyType::KeyMetakom:
        return "Metakom";
        break;
    case iButtonKeyType::KeyDallas:
        return "Dallas";
        break;
    default:
        furi_crash("Invalid iButton type");
        return "";
        break;
    }
}

bool iButtonKey::get_key_type_by_type_string(const char* type_string, iButtonKeyType* key_type) {
    if(strcmp(type_string, get_key_type_string_by_type(iButtonKeyType::KeyCyfral)) == 0) {
        *key_type = iButtonKeyType::KeyCyfral;
    } else if(strcmp(type_string, get_key_type_string_by_type(iButtonKeyType::KeyMetakom)) == 0) {
        *key_type = iButtonKeyType::KeyMetakom;
    } else if(strcmp(type_string, get_key_type_string_by_type(iButtonKeyType::KeyDallas)) == 0) {
        *key_type = iButtonKeyType::KeyDallas;
    } else {
        return false;
    }

    return true;
}

uint8_t iButtonKey::get_type_data_size_by_type(iButtonKeyType key_type) {
    uint8_t size = 0;

    switch(key_type) {
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

iButtonKey::iButtonKey() {
}
