#include "bad_usb_hid.h"
#include <extra_profiles/hid_profile.h>
#include <storage/storage.h>

#define TAG "BadUSB HID"

void* hid_usb_init(FuriHalUsbHidConfig* hid_cfg) {
    furi_check(furi_hal_usb_set_config(&usb_hid, hid_cfg));
    return NULL;
}

void hid_usb_deinit(void* inst) {
    UNUSED(inst);
    furi_check(furi_hal_usb_set_config(NULL, NULL));
}

void hid_usb_set_state_callback(void* inst, HidStateCallback cb, void* context) {
    UNUSED(inst);
    furi_hal_hid_set_state_callback(cb, context);
}

bool hid_usb_is_connected(void* inst) {
    UNUSED(inst);
    return furi_hal_hid_is_connected();
}

bool hid_usb_kb_press(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_kb_press(button);
}

bool hid_usb_kb_release(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_kb_release(button);
}

bool hid_usb_consumer_press(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_consumer_key_press(button);
}

bool hid_usb_consumer_release(void* inst, uint16_t button) {
    UNUSED(inst);
    return furi_hal_hid_consumer_key_release(button);
}

bool hid_usb_release_all(void* inst) {
    UNUSED(inst);
    bool state = furi_hal_hid_kb_release_all();
    state &= furi_hal_hid_consumer_key_release_all();
    return state;
}

uint8_t hid_usb_get_led_state(void* inst) {
    UNUSED(inst);
    return furi_hal_hid_get_led_state();
}

static const BadUsbHidApi hid_api_usb = {
    .init = hid_usb_init,
    .deinit = hid_usb_deinit,
    .set_state_callback = hid_usb_set_state_callback,
    .is_connected = hid_usb_is_connected,

    .kb_press = hid_usb_kb_press,
    .kb_release = hid_usb_kb_release,
    .consumer_press = hid_usb_consumer_press,
    .consumer_release = hid_usb_consumer_release,
    .release_all = hid_usb_release_all,
    .get_led_state = hid_usb_get_led_state,
};
const BadUsbHidApi* bad_usb_hid_get_interface() {
    return &hid_api_usb;
}
