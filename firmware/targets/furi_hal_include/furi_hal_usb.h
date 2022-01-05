#pragma once

#include "usb.h"

typedef struct UsbInterface UsbInterface;

struct UsbInterface {
    void (*init)(usbd_device* dev, UsbInterface* intf);
    void (*deinit)(usbd_device* dev);
    void (*wakeup)(usbd_device* dev);
    void (*suspend)(usbd_device* dev);

    struct usb_device_descriptor* dev_descr;

    void* str_manuf_descr;
    void* str_prod_descr;
    void* str_serial_descr;

    void* cfg_descr;
};

/** USB device interface modes */
extern UsbInterface usb_cdc_single;
extern UsbInterface usb_cdc_dual;
extern UsbInterface usb_hid;
extern UsbInterface usb_hid_u2f;

/** USB device low-level initialization
 */
void furi_hal_usb_init();

/** Set USB device configuration
 *
 * @param      mode new USB device mode
 */
void furi_hal_usb_set_config(UsbInterface* new_if);

/** Get USB device configuration
 *
 * @return    current USB device mode
 */
UsbInterface* furi_hal_usb_get_config();

/** Disable USB device
 */
void furi_hal_usb_disable();

/** Enable USB device
 */
void furi_hal_usb_enable();
