#include "hex.h"

bool hex_char_to_hex_nibble(char c, uint8_t* nibble) {
    if((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
        if((c >= '0' && c <= '9')) {
            *nibble = c - '0';
        } else if((c >= 'A' && c <= 'F')) {
            *nibble = c - 'A' + 10;
        } else {
            *nibble = c - 'a' + 10;
        }
        return true;
    } else {
        return false;
    }
}

bool hex_char_to_uint8(char hi, char low, uint8_t* value) {
    uint8_t hi_nibble_value, low_nibble_value;

    if(hex_char_to_hex_nibble(hi, &hi_nibble_value) &&
       hex_char_to_hex_nibble(low, &low_nibble_value)) {
        *value = (hi_nibble_value << 4) | low_nibble_value;
        return true;
    } else {
        return false;
    }
}

bool hex_chars_to_uint8(const char* value_str, uint8_t* value) {
    bool parse_success = false;
    while(*value_str && value_str[1]) {
        parse_success = hex_char_to_uint8(*value_str, value_str[1], value++);
        if(!parse_success) break;
        value_str += 2;
    }
    return parse_success;
}

bool hex_chars_to_uint64(const char* value_str, uint64_t* value) {
    uint8_t* _value = (uint8_t*)value;
    bool parse_success = false;

    for(uint8_t i = 0; i < 8; i++) {
        parse_success = hex_char_to_uint8(value_str[i * 2], value_str[i * 2 + 1], &_value[7 - i]);
        if(!parse_success) break;
    }
    return parse_success;
}

void uint8_to_hex_chars(const uint8_t* src, uint8_t* target, int length) {
    const char chars[] = "0123456789ABCDEF";
    while(--length >= 0)
        target[length] = chars[(src[length >> 1] >> ((1 - (length & 1)) << 2)) & 0xF];
}
