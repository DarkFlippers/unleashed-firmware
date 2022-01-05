#include "dialog_ex_vm.h"

DialogExVM::DialogExVM() {
    dialog_ex = dialog_ex_alloc();
}

DialogExVM::~DialogExVM() {
    dialog_ex_free(dialog_ex);
}

View* DialogExVM::get_view() {
    return dialog_ex_get_view(dialog_ex);
}

void DialogExVM::clean() {
    set_result_callback(NULL);
    set_context(NULL);
    set_header(NULL, 0, 0, AlignLeft, AlignBottom);
    set_text(NULL, 0, 0, AlignLeft, AlignBottom);
    set_icon(0, 0, NULL);
    set_left_button_text(NULL);
    set_center_button_text(NULL);
    set_right_button_text(NULL);
}

void DialogExVM::set_result_callback(DialogExResultCallback callback) {
    dialog_ex_set_result_callback(dialog_ex, callback);
}

void DialogExVM::set_context(void* context) {
    dialog_ex_set_context(dialog_ex, context);
}

void DialogExVM::set_header(
    const char* text,
    uint8_t x,
    uint8_t y,
    Align horizontal,
    Align vertical) {
    dialog_ex_set_header(dialog_ex, text, x, y, horizontal, vertical);
}

void DialogExVM::set_text(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) {
    dialog_ex_set_text(dialog_ex, text, x, y, horizontal, vertical);
}

void DialogExVM::set_icon(uint8_t x, uint8_t y, const Icon* icon) {
    dialog_ex_set_icon(dialog_ex, x, y, icon);
}

void DialogExVM::set_left_button_text(const char* text) {
    dialog_ex_set_left_button_text(dialog_ex, text);
}

void DialogExVM::set_center_button_text(const char* text) {
    dialog_ex_set_center_button_text(dialog_ex, text);
}

void DialogExVM::set_right_button_text(const char* text) {
    dialog_ex_set_right_button_text(dialog_ex, text);
}
