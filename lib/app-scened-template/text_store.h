#pragma once
#include <stdint.h>

class TextStore {
public:
    TextStore(uint8_t text_size);
    ~TextStore();

    void set(const char* text...);
    const uint8_t text_size;
    char* text;
};
