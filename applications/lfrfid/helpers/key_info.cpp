#include "key_info.h"
#include <string.h>

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
    case LfrfidKeyType::KeyIoProxXSF:
        return "IoProxXSF";
        break;
    }

    return "Unknown";
}

const char* lfrfid_key_get_manufacturer_string(LfrfidKeyType type) {
    switch(type) {
    case LfrfidKeyType::KeyEM4100:
        return "EM-Marin";
        break;
    case LfrfidKeyType::KeyH10301:
        return "HID";
        break;
    case LfrfidKeyType::KeyI40134:
        return "Indala";
        break;
    case LfrfidKeyType::KeyIoProxXSF:
        return "Kantech";
    }

    return "Unknown";
}

bool lfrfid_key_get_string_type(const char* string, LfrfidKeyType* type) {
    bool result = true;

    if(strcmp("EM4100", string) == 0) {
        *type = LfrfidKeyType::KeyEM4100;
    } else if(strcmp("H10301", string) == 0) {
        *type = LfrfidKeyType::KeyH10301;
    } else if(strcmp("I40134", string) == 0) {
        *type = LfrfidKeyType::KeyI40134;
    } else if(strcmp("IoProxXSF", string) == 0) {
        *type = LfrfidKeyType::KeyIoProxXSF;
    } else {
        result = false;
    }

    return result;
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
    case LfrfidKeyType::KeyIoProxXSF:
        return 4;
        break;
    }

    return 0;
}
