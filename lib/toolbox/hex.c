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