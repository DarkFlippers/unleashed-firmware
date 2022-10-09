#pragma once

#include <gui/view.h>

typedef struct UsbHidMouse UsbHidMouse;

UsbHidMouse* usb_hid_mouse_alloc();

void usb_hid_mouse_free(UsbHidMouse* usb_hid_mouse);

View* usb_hid_mouse_get_view(UsbHidMouse* usb_hid_mouse);

void usb_hid_mouse_set_connected_status(UsbHidMouse* usb_hid_mouse, bool connected);
