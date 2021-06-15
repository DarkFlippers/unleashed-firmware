#include "text-input-vm.h"

TextInputVM::TextInputVM() {
    text_input = text_input_alloc();
}

TextInputVM::~TextInputVM() {
    text_input_free(text_input);
}

View* TextInputVM::get_view() {
    return text_input_get_view(text_input);
}

void TextInputVM::clean() {
    set_result_callback(NULL, NULL, NULL, 0);
    set_header_text("");
}

void TextInputVM::set_result_callback(
    TextInputCallback callback,
    void* callback_context,
    char* text,
    uint8_t max_text_length) {
    text_input_set_result_callback(text_input, callback, callback_context, text, max_text_length);
}

void TextInputVM::set_header_text(const char* text) {
    text_input_set_header_text(text_input, text);
}
