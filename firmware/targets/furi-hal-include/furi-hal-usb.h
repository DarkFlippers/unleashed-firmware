#pragma once

#include "usb.h"

typedef enum {
    UsbModeNone,
    UsbModeVcpSingle,
    UsbModeVcpDual,
    UsbModeHid,
    UsbModeU2F,

    UsbModesNum,
} UsbMode;

void furi_hal_usb_init();

void furi_hal_usb_set_config(UsbMode mode);

void furi_hal_usb_disable();

void furi_hal_usb_enable();
