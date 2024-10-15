#pragma once

#include <gui/view.h>
#include "../helpers/ducky_script.h"

typedef struct BadUsb BadUsb;
typedef void (*BadUsbButtonCallback)(InputKey key, void* context);

BadUsb* bad_usb_view_alloc(void);

void bad_usb_view_free(BadUsb* bad_usb);

View* bad_usb_view_get_view(BadUsb* bad_usb);

void bad_usb_view_set_button_callback(
    BadUsb* bad_usb,
    BadUsbButtonCallback callback,
    void* context);

void bad_usb_view_set_file_name(BadUsb* bad_usb, const char* name);

void bad_usb_view_set_layout(BadUsb* bad_usb, const char* layout);

void bad_usb_view_set_state(BadUsb* bad_usb, BadUsbState* st);

void bad_usb_view_set_interface(BadUsb* bad_usb, BadUsbHidInterface interface);

bool bad_usb_view_is_idle_state(BadUsb* bad_usb);
