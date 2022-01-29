#include "text_input_vm.h"

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
    text_input_reset(text_input);
}

void TextInputVM::set_result_callback(
    TextInputCallback callback,
    void* callback_context,
    char* text,
    uint8_t max_text_length,
    bool clear_default_text) {
    text_input_set_result_callback(
        text_input, callback, callback_context, text, max_text_length, clear_default_text);
}

void TextInputVM::set_header_text(const char* text) {
    text_input_set_header_text(text_input, text);
}

void TextInputVM::set_validator(TextInputValidatorCallback callback, void* callback_context) {
    text_input_set_validator(text_input, callback, callback_context);
}

void* TextInputVM::get_validator_callback_context() {
    return text_input_get_validator_callback_context(text_input);
}
