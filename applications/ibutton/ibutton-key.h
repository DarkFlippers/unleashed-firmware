#pragma once
#include <stdint.h>
#include "helpers/key-info.h"

class iButtonKey {
public:
    uint8_t get_size();

    void set_data(uint8_t* data, uint8_t data_count);
    uint8_t* get_data();
    uint8_t get_type_data_size();

    void set_name(const char* name);
    const char* get_name();

    void set_type(iButtonKeyType key_type);
    iButtonKeyType get_key_type();

    // temporary constructor for KeyStore mockup
    iButtonKey(
        iButtonKeyType type,
        const char* name,
        uint8_t d0,
        uint8_t d1,
        uint8_t d2,
        uint8_t d3,
        uint8_t d4,
        uint8_t d5,
        uint8_t d6,
        uint8_t d7);

    iButtonKey();

private:
    uint8_t data[IBUTTON_KEY_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0};
    const char* name = {0};

    iButtonKeyType type = iButtonKeyType::KeyDallas;
};