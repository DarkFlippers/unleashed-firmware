#include "byte_input_vm.h"

ByteInputVM::ByteInputVM() {
    byte_input = byte_input_alloc();
}

ByteInputVM::~ByteInputVM() {
    byte_input_free(byte_input);
}

View* ByteInputVM::get_view() {
    return byte_input_get_view(byte_input);
}

void ByteInputVM::clean() {
    byte_input_set_header_text(byte_input, "");
    byte_input_set_result_callback(byte_input, NULL, NULL, NULL, NULL, 0);
}

void ByteInputVM::set_result_callback(
    ByteInputCallback input_callback,
    ByteChangedCallback changed_callback,
    void* callback_context,
    uint8_t* bytes,
    uint8_t bytes_count) {
    byte_input_set_result_callback(
        byte_input, input_callback, changed_callback, callback_context, bytes, bytes_count);
}

void ByteInputVM::set_header_text(const char* text) {
    byte_input_set_header_text(byte_input, text);
}
