#pragma once

#include <gui/view.h>

typedef struct UsbHidDirpad UsbHidDirpad;

UsbHidDirpad* usb_hid_dirpad_alloc();

void usb_hid_dirpad_free(UsbHidDirpad* usb_hid_dirpad);

View* usb_hid_dirpad_get_view(UsbHidDirpad* usb_hid_dirpad);

void usb_hid_dirpad_set_connected_status(UsbHidDirpad* usb_hid_dirpad, bool connected);
