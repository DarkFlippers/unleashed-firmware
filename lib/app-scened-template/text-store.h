#pragma once
#include <stdint.h>

class TextStore {
public:
    TextStore(uint8_t text_size);
    ~TextStore();

    void set_text_store(const char* text...);
    const uint8_t text_size;
    char* text;
};