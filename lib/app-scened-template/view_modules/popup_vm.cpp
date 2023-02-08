#include "popup_vm.h"
#include <gui/modules/popup.h>

PopupVM::PopupVM() {
    popup = popup_alloc();
}

PopupVM::~PopupVM() {
    popup_free(popup);
}

View* PopupVM::get_view() {
    return popup_get_view(popup);
}

void PopupVM::clean() {
    set_callback(NULL);
    set_context(NULL);
    set_header(NULL, 0, 0, AlignLeft, AlignBottom);
    set_text(NULL, 0, 0, AlignLeft, AlignBottom);
    set_icon(0, 0, NULL);
    disable_timeout();
    set_timeout(1000);
}

void PopupVM::set_callback(PopupCallback callback) {
    popup_set_callback(popup, callback);
}

void PopupVM::set_context(void* context) {
    popup_set_context(popup, context);
}

void PopupVM::set_header(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) {
    popup_set_header(popup, text, x, y, horizontal, vertical);
}

void PopupVM::set_text(const char* text, uint8_t x, uint8_t y, Align horizontal, Align vertical) {
    popup_set_text(popup, text, x, y, horizontal, vertical);
}

void PopupVM::set_icon(int8_t x, int8_t y, const Icon* icon) {
    popup_set_icon(popup, x, y, icon);
}

void PopupVM::set_timeout(uint32_t timeout_in_ms) {
    popup_set_timeout(popup, timeout_in_ms);
}

void PopupVM::enable_timeout() {
    popup_enable_timeout(popup);
}

void PopupVM::disable_timeout() {
    popup_disable_timeout(popup);
}
