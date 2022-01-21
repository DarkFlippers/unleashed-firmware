#include "submenu_vm.h"

SubmenuVM::SubmenuVM() {
    submenu = submenu_alloc();
}

SubmenuVM::~SubmenuVM() {
    submenu_free(submenu);
}

View* SubmenuVM::get_view() {
    return submenu_get_view(submenu);
}

void SubmenuVM::clean() {
    submenu_reset(submenu);
}

void SubmenuVM::add_item(
    const char* label,
    uint32_t index,
    SubmenuItemCallback callback,
    void* callback_context) {
    submenu_add_item(submenu, label, index, callback, callback_context);
}

void SubmenuVM::set_selected_item(uint32_t index) {
    submenu_set_selected_item(submenu, index);
}

void SubmenuVM::set_header(const char* header) {
    submenu_set_header(submenu, header);
}
