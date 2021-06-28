#include "key-info.h"

const char* lfrfid_key_get_type_string(LfrfidKeyType type) {
    switch(type) {
    case LfrfidKeyType::KeyEM4100:
        return "EM4100";
        break;
    case LfrfidKeyType::KeyH10301:
        return "H10301";
        break;
    case LfrfidKeyType::KeyI40134:
        return "I40134";
        break;
    }

    return "Unknown";
}

uint8_t lfrfid_key_get_type_data_count(LfrfidKeyType type) {
    switch(type) {
    case LfrfidKeyType::KeyEM4100:
        return 5;
        break;
    case LfrfidKeyType::KeyH10301:
        return 3;
        break;
    case LfrfidKeyType::KeyI40134:
        return 3;
        break;
    }

    return 0;
}