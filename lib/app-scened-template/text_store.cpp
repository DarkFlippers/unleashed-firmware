#include "text_store.h"
#include <furi.h>

TextStore::TextStore(uint8_t _text_size)
    : text_size(_text_size) {
    text = static_cast<char*>(malloc(text_size + 1));
}

TextStore::~TextStore() {
    free(text);
}

void TextStore::set(const char* _text...) {
    va_list args;
    va_start(args, _text);
    vsnprintf(text, text_size, _text, args);
    va_end(args);
}
