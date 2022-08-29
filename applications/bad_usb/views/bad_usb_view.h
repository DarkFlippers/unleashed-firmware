#pragma once

#include <gui/view.h>
#include "../bad_usb_script.h"

typedef struct BadUsb BadUsb;
typedef void (*BadUsbOkCallback)(InputType type, void* context);

BadUsb* bad_usb_alloc();

void bad_usb_free(BadUsb* bad_usb);

View* bad_usb_get_view(BadUsb* bad_usb);

void bad_usb_set_ok_callback(BadUsb* bad_usb, BadUsbOkCallback callback, void* context);

void bad_usb_set_file_name(BadUsb* bad_usb, const char* name);

void bad_usb_set_state(BadUsb* bad_usb, BadUsbState* st);
