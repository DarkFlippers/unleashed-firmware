#pragma once
#include <stdint.h>
#include <list>
#include "key-info.h"
#include "../ibutton-key.h"

class KeyStore {
public:
    uint16_t get_key_count();

    uint8_t add_key();

    void set_key_type(uint8_t index, iButtonKeyType type);
    void set_key_name(uint8_t index, char* name);
    void set_key_data(uint8_t index, uint8_t* data, uint8_t data_size);

    iButtonKeyType get_key_type(uint8_t index);
    const char* get_key_name(uint8_t index);
    uint8_t* get_key_data(uint8_t index);

    void remove_key(uint8_t index);

    KeyStore();
    ~KeyStore();

private:
    std::list<iButtonKey> store;
    iButtonKey* get_key(uint8_t index);
};