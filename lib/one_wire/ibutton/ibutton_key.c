#include <furi.h>
#include <one_wire/maxim_crc.h>
#include "ibutton_key.h"

struct iButtonKey {
    uint8_t data[IBUTTON_KEY_DATA_SIZE];
    iButtonKeyType type;
};

iButtonKey* ibutton_key_alloc() {
    iButtonKey* key = malloc(sizeof(iButtonKey));
    memset(key, 0, sizeof(iButtonKey));
    return key;
}

void ibutton_key_free(iButtonKey* key) {
    free(key);
}

void ibutton_key_set(iButtonKey* to, const iButtonKey* from) {
    memcpy(to, from, sizeof(iButtonKey));
}

void ibutton_key_set_data(iButtonKey* key, uint8_t* data, uint8_t data_count) {
    furi_check(data_count > 0);
    furi_check(data_count <= IBUTTON_KEY_DATA_SIZE);

    memset(key->data, 0, IBUTTON_KEY_DATA_SIZE);
    memcpy(key->data, data, data_count);
}

void ibutton_key_clear_data(iButtonKey* key) {
    memset(key->data, 0, IBUTTON_KEY_DATA_SIZE);
}

const uint8_t* ibutton_key_get_data_p(iButtonKey* key) {
    return key->data;
}

uint8_t ibutton_key_get_data_size(iButtonKey* key) {
    return ibutton_key_get_size_by_type(key->type);
}

void ibutton_key_set_type(iButtonKey* key, iButtonKeyType key_type) {
    key->type = key_type;
}

iButtonKeyType ibutton_key_get_type(iButtonKey* key) {
    return key->type;
}

const char* ibutton_key_get_string_by_type(iButtonKeyType key_type) {
    switch(key_type) {
    case iButtonKeyCyfral:
        return "Cyfral";
        break;
    case iButtonKeyMetakom:
        return "Metakom";
        break;
    case iButtonKeyDS1990:
        return "Dallas";
        break;
    default:
        furi_crash("Invalid iButton type");
        return "";
        break;
    }
}

bool ibutton_key_get_type_by_string(const char* type_string, iButtonKeyType* key_type) {
    if(strcmp(type_string, ibutton_key_get_string_by_type(iButtonKeyCyfral)) == 0) {
        *key_type = iButtonKeyCyfral;
    } else if(strcmp(type_string, ibutton_key_get_string_by_type(iButtonKeyMetakom)) == 0) {
        *key_type = iButtonKeyMetakom;
    } else if(strcmp(type_string, ibutton_key_get_string_by_type(iButtonKeyDS1990)) == 0) {
        *key_type = iButtonKeyDS1990;
    } else {
        return false;
    }

    return true;
}

uint8_t ibutton_key_get_size_by_type(iButtonKeyType key_type) {
    uint8_t size = 0;

    switch(key_type) {
    case iButtonKeyCyfral:
        size = 2;
        break;
    case iButtonKeyMetakom:
        size = 4;
        break;
    case iButtonKeyDS1990:
        size = 8;
        break;
    }

    return size;
}

uint8_t ibutton_key_get_max_size() {
    return IBUTTON_KEY_DATA_SIZE;
}

bool ibutton_key_dallas_crc_is_valid(iButtonKey* key) {
    return (maxim_crc8(key->data, 8, MAXIM_CRC8_INIT) == 0);
}

bool ibutton_key_dallas_is_1990_key(iButtonKey* key) {
    return (key->data[0] == 0x01);
}
