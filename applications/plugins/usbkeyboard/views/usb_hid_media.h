#pragma once

#include <gui/view.h>

typedef struct UsbHidMedia UsbHidMedia;

UsbHidMedia* usb_hid_media_alloc();

void usb_hid_media_free(UsbHidMedia* usb_hid_media);

View* usb_hid_media_get_view(UsbHidMedia* usb_hid_media);

void usb_hid_media_set_connected_status(UsbHidMedia* usb_hid_media, bool connected);
