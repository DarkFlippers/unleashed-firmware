#include "hid.h"

#ifndef HID_TRANSPORT_USB
#error "HID_TRANSPORT_USB must be defined"
#endif

void hid_hal_keyboard_press(Hid* instance, uint16_t event) {
    furi_assert(instance);
    furi_hal_hid_kb_press(event);
}

void hid_hal_keyboard_release(Hid* instance, uint16_t event) {
    furi_assert(instance);
    furi_hal_hid_kb_release(event);
}

void hid_hal_keyboard_release_all(Hid* instance) {
    furi_assert(instance);
    furi_hal_hid_kb_release_all();
}

void hid_hal_consumer_key_press(Hid* instance, uint16_t event) {
    furi_assert(instance);
    furi_hal_hid_consumer_key_press(event);
}

void hid_hal_consumer_key_release(Hid* instance, uint16_t event) {
    furi_assert(instance);
    furi_hal_hid_consumer_key_release(event);
}

void hid_hal_consumer_key_release_all(Hid* instance) {
    furi_assert(instance);
    furi_hal_hid_kb_release_all();
}

void hid_hal_mouse_move(Hid* instance, int8_t dx, int8_t dy) {
    furi_assert(instance);
    furi_hal_hid_mouse_move(dx, dy);
}

void hid_hal_mouse_scroll(Hid* instance, int8_t delta) {
    furi_assert(instance);
    furi_hal_hid_mouse_scroll(delta);
}

void hid_hal_mouse_press(Hid* instance, uint16_t event) {
    furi_assert(instance);
    furi_hal_hid_mouse_press(event);
}

void hid_hal_mouse_release(Hid* instance, uint16_t event) {
    furi_assert(instance);
    furi_hal_hid_mouse_release(event);
}

void hid_hal_mouse_release_all(Hid* instance) {
    furi_assert(instance);
    furi_hal_hid_mouse_release(HID_MOUSE_BTN_LEFT);
    furi_hal_hid_mouse_release(HID_MOUSE_BTN_RIGHT);
}
