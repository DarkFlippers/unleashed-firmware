#pragma once

#include <gui/view.h>

typedef struct UsbHidKeyboard UsbHidKeyboard;

UsbHidKeyboard* usb_hid_keyboard_alloc();

void usb_hid_keyboard_free(UsbHidKeyboard* usb_hid_keyboard);

View* usb_hid_keyboard_get_view(UsbHidKeyboard* usb_hid_keyboard);

void usb_hid_keyboard_set_connected_status(UsbHidKeyboard* usb_hid_keyboard, bool connected);
