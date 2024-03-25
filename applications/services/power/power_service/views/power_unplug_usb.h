#pragma once

typedef struct PowerUnplugUsb PowerUnplugUsb;

#include <gui/view.h>

PowerUnplugUsb* power_unplug_usb_alloc(void);

void power_unplug_usb_free(PowerUnplugUsb* power_unplug_usb);

View* power_unplug_usb_get_view(PowerUnplugUsb* power_unplug_usb);
