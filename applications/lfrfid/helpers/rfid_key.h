#pragma once
#include "key_info.h"
#include <array>

class RfidKey {
public:
    RfidKey();
    ~RfidKey();

    void set_type(LfrfidKeyType type);
    void set_data(const uint8_t* data, const uint8_t data_size);
    void set_name(const char* name);

    LfrfidKeyType get_type();
    const uint8_t* get_data();
    const char* get_type_text();
    uint8_t get_type_data_count() const;
    char* get_name();
    uint8_t get_name_length();
    void clear();
    RfidKey& operator=(const RfidKey& rhs);

private:
    std::array<uint8_t, LFRFID_KEY_SIZE> data;
    LfrfidKeyType type;
    char name[LFRFID_KEY_NAME_SIZE + 1];
};
