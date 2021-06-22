#include "args.h"

size_t args_get_first_word_length(string_t args) {
    size_t ws = string_search_char(args, ' ');
    if(ws == STRING_FAILURE) {
        ws = string_size(args);
    }

    return ws;
}

size_t args_length(string_t args) {
    return string_size(args);
}

bool args_read_string_and_trim(string_t args, string_t word) {
    size_t cmd_length = args_get_first_word_length(args);

    if(cmd_length == 0) {
        return false;
    }

    string_set_n(word, args, 0, cmd_length);
    string_right(args, cmd_length);
    string_strim(args);

    return true;
}

bool args_char_to_hex_nibble(char c, uint8_t* nibble) {
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

bool args_char_to_hex(char hi_nibble, char low_nibble, uint8_t* byte) {
    uint8_t hi_nibble_value = 0;
    uint8_t low_nibble_value = 0;
    bool result = false;

    if(args_char_to_hex_nibble(hi_nibble, &hi_nibble_value)) {
        if(args_char_to_hex_nibble(low_nibble, &low_nibble_value)) {
            result = true;
            *byte = (hi_nibble_value << 4) | low_nibble_value;
        }
    }

    return result;
}

bool args_read_hex_bytes(string_t args, uint8_t* bytes, uint8_t bytes_count) {
    bool result = true;
    const char* str_pointer = string_get_cstr(args);

    if(args_get_first_word_length(args) == (bytes_count * 2)) {
        for(uint8_t i = 0; i < bytes_count; i++) {
            if(!args_char_to_hex(str_pointer[i * 2], str_pointer[i * 2 + 1], &(bytes[i]))) {
                result = false;
                break;
            }
        }
    } else {
        result = false;
    }

    return result;
}