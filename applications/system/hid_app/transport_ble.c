#include "hid.h"

#ifndef HID_TRANSPORT_BLE
#error "HID_TRANSPORT_BLE must be defined"
#endif

void hid_hal_keyboard_press(Hid* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_kb_press(instance->ble_hid_profile, event);
}

void hid_hal_keyboard_release(Hid* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_kb_release(instance->ble_hid_profile, event);
}

void hid_hal_keyboard_release_all(Hid* instance) {
    furi_assert(instance);
    ble_profile_hid_kb_release_all(instance->ble_hid_profile);
}

void hid_hal_consumer_key_press(Hid* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_consumer_key_press(instance->ble_hid_profile, event);
}

void hid_hal_consumer_key_release(Hid* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_consumer_key_release(instance->ble_hid_profile, event);
}

void hid_hal_consumer_key_release_all(Hid* instance) {
    furi_assert(instance);
    ble_profile_hid_consumer_key_release_all(instance->ble_hid_profile);
}

void hid_hal_mouse_move(Hid* instance, int8_t dx, int8_t dy) {
    furi_assert(instance);
    ble_profile_hid_mouse_move(instance->ble_hid_profile, dx, dy);
}

void hid_hal_mouse_scroll(Hid* instance, int8_t delta) {
    furi_assert(instance);
    ble_profile_hid_mouse_scroll(instance->ble_hid_profile, delta);
}

void hid_hal_mouse_press(Hid* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_mouse_press(instance->ble_hid_profile, event);
}

void hid_hal_mouse_release(Hid* instance, uint16_t event) {
    furi_assert(instance);
    ble_profile_hid_mouse_release(instance->ble_hid_profile, event);
}

void hid_hal_mouse_release_all(Hid* instance) {
    furi_assert(instance);
    ble_profile_hid_mouse_release_all(instance->ble_hid_profile);
}
