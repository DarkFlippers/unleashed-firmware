#pragma once
#include <stdint.h>
#include "helpers/key-info.h"

class iButtonKey {
public:
    uint8_t get_size();

    void set_data(uint8_t* data, uint8_t data_count);
    void clear_data();
    uint8_t* get_data();
    uint8_t get_type_data_size();

    void set_name(const char* name);
    char* get_name();

    void set_type(iButtonKeyType key_type);
    iButtonKeyType get_key_type();

    iButtonKey();

private:
    uint8_t data[IBUTTON_KEY_DATA_SIZE] = {0, 0, 0, 0, 0, 0, 0, 0};
    char name[IBUTTON_KEY_NAME_SIZE] = {0};

    iButtonKeyType type = iButtonKeyType::KeyDallas;
};