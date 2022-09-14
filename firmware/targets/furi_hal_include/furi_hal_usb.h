#pragma once

#include "usb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FuriHalUsbInterface FuriHalUsbInterface;

struct FuriHalUsbInterface {
    void (*init)(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx);
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
extern FuriHalUsbInterface usb_cdc_single;
extern FuriHalUsbInterface usb_cdc_dual;
extern FuriHalUsbInterface usb_hid;
extern FuriHalUsbInterface usb_hid_u2f;

typedef enum {
    FuriHalUsbStateEventReset,
    FuriHalUsbStateEventWakeup,
    FuriHalUsbStateEventSuspend,
    FuriHalUsbStateEventDescriptorRequest,
} FuriHalUsbStateEvent;

typedef void (*FuriHalUsbStateCallback)(FuriHalUsbStateEvent state, void* context);

/** USB device low-level initialization
 */
void furi_hal_usb_init();

/** Set USB device configuration
 *
 * @param      mode new USB device mode
 * @param      ctx context passed to device mode init function
 * @return     true - mode switch started, false - mode switch is locked
 */
bool furi_hal_usb_set_config(FuriHalUsbInterface* new_if, void* ctx);

/** Get USB device configuration
 *
 * @return    current USB device mode
 */
FuriHalUsbInterface* furi_hal_usb_get_config();

/** Lock USB device mode switch
 */
void furi_hal_usb_lock();

/** Unlock USB device mode switch
 */
void furi_hal_usb_unlock();

/** Check if USB device mode switch locked
 * 
 * @return    lock state
 */
bool furi_hal_usb_is_locked();

/** Disable USB device
 */
void furi_hal_usb_disable();

/** Enable USB device
 */
void furi_hal_usb_enable();

/** Set USB state callback
 */
void furi_hal_usb_set_state_callback(FuriHalUsbStateCallback cb, void* ctx);

/** Restart USB device
 */
void furi_hal_usb_reinit();

#ifdef __cplusplus
}
#endif
