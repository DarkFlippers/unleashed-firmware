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

bool hex_chars_to_uint8(char hi, char low, uint8_t* value) {
    uint8_t hi_nibble_value, low_nibble_value;

    if(hex_char_to_hex_nibble(hi, &hi_nibble_value) &&
       hex_char_to_hex_nibble(low, &low_nibble_value)) {
        *value = (hi_nibble_value << 4) | low_nibble_value;
        return true;
    } else {
        return false;
    }
}