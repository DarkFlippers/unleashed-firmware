#pragma once

#include <gui/view.h>
#include "../bad_usb_script.h"

typedef struct BadUsb BadUsb;
typedef void (*BadUsbButtonCallback)(InputKey key, void* context);

BadUsb* bad_usb_alloc();

void bad_usb_free(BadUsb* bad_usb);

View* bad_usb_get_view(BadUsb* bad_usb);

void bad_usb_set_button_callback(BadUsb* bad_usb, BadUsbButtonCallback callback, void* context);

void bad_usb_set_file_name(BadUsb* bad_usb, const char* name);

void bad_usb_set_layout(BadUsb* bad_usb, const char* layout);

void bad_usb_set_state(BadUsb* bad_usb, BadUsbState* st);

bool bad_usb_is_idle_state(BadUsb* bad_usb);
