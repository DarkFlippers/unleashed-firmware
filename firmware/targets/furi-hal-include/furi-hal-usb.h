#pragma once

#include "usb.h"

/** USB device modes */
typedef enum {
    UsbModeNone,
    UsbModeVcpSingle,
    UsbModeVcpDual,
    UsbModeHid,
    UsbModeU2F,

    UsbModesNum,
} UsbMode;

/** USB device low-level initialization
 */
void furi_hal_usb_init();

/** Set USB device configuration
 *
 * @param      mode new USB device mode
 */
void furi_hal_usb_set_config(UsbMode mode);

/** Get USB device configuration
 *
 * @return    current USB device mode
 */
UsbMode furi_hal_usb_get_config();

/** Disable USB device
 */
void furi_hal_usb_disable();

/** Enable USB device
 */
void furi_hal_usb_enable();
