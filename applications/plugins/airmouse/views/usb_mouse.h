#pragma once

#include <gui/view.h>
#include <gui/view_dispatcher.h>

typedef struct UsbMouse UsbMouse;

UsbMouse* usb_mouse_alloc(ViewDispatcher* view_dispatcher);

void usb_mouse_free(UsbMouse* usb_mouse);

View* usb_mouse_get_view(UsbMouse* usb_mouse);
