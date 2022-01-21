#include "furi_hal_version.h"
#include "furi_hal_usb_i.h"
#include "furi_hal_usb.h"
#include <furi.h>

#include "usb.h"

#define TAG "FuriHalUsb"

#define USB_RECONNECT_DELAY 500

static FuriHalUsbInterface* usb_if_cur;
static FuriHalUsbInterface* usb_if_next;

static const struct usb_string_descriptor dev_lang_desc = USB_ARRAY_DESC(USB_LANGID_ENG_US);

static uint32_t ubuf[0x20];
usbd_device udev;

static FuriHalUsbStateCallback callback;
static void* cb_ctx;

static usbd_respond usb_descriptor_get(usbd_ctlreq* req, void** address, uint16_t* length);
static void reset_evt(usbd_device* dev, uint8_t event, uint8_t ep);
static void susp_evt(usbd_device* dev, uint8_t event, uint8_t ep);
static void wkup_evt(usbd_device* dev, uint8_t event, uint8_t ep);

struct UsbCfg {
    osTimerId_t reconnect_tmr;
    bool enabled;
    bool connected;
    bool mode_changing;
} usb_config;

static void furi_hal_usb_tmr_cb(void* context);

/* Low-level init */
void furi_hal_usb_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_PWR_EnableVddUSB();

    GPIO_InitStruct.Pin = LL_GPIO_PIN_11 | LL_GPIO_PIN_12;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_10;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    usbd_init(&udev, &usbd_hw, USB_EP0_SIZE, ubuf, sizeof(ubuf));
    usbd_enable(&udev, true);

    usbd_reg_descr(&udev, usb_descriptor_get);
    usbd_reg_event(&udev, usbd_evt_susp, susp_evt);
    usbd_reg_event(&udev, usbd_evt_wkup, wkup_evt);
    // Reset callback will be enabled after first mode change to avoid getting false reset events

    usb_config.enabled = false;
    usb_config.reconnect_tmr = NULL;
    HAL_NVIC_SetPriority(USB_LP_IRQn, 5, 0);
    NVIC_EnableIRQ(USB_LP_IRQn);

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_usb_set_config(FuriHalUsbInterface* new_if) {
    if((new_if != usb_if_cur) && (usb_config.enabled)) { // Interface mode change - first stage
        usb_config.mode_changing = true;
        usb_if_next = new_if;
        if(usb_config.reconnect_tmr == NULL)
            usb_config.reconnect_tmr = osTimerNew(furi_hal_usb_tmr_cb, osTimerOnce, NULL, NULL);
        furi_hal_usb_disable();
        usb_config.mode_changing = true;
        osTimerStart(usb_config.reconnect_tmr, USB_RECONNECT_DELAY);
    } else if(
        (usb_config.mode_changing) &&
        (usb_if_next != new_if)) { // Last interface mode change wasn't completed
        osTimerStop(usb_config.reconnect_tmr);
        usb_if_next = new_if;
        osTimerStart(usb_config.reconnect_tmr, USB_RECONNECT_DELAY);
    } else { // Interface mode change - second stage
        if(usb_if_cur != NULL) usb_if_cur->deinit(&udev);
        if(new_if != NULL) {
            new_if->init(&udev, new_if);
            usbd_reg_event(&udev, usbd_evt_reset, reset_evt);
            FURI_LOG_I(TAG, "USB Mode change done");
            usb_config.enabled = true;
            usb_if_cur = new_if;
            usb_config.mode_changing = false;
        }
    }
}

void furi_hal_usb_reinit() {
    // Temporary disable callback to avoid getting false reset events
    usbd_reg_event(&udev, usbd_evt_reset, NULL);
    FURI_LOG_I(TAG, "USB Reinit");
    furi_hal_usb_disable();
    usbd_enable(&udev, false);
    usbd_enable(&udev, true);
    if(usb_config.reconnect_tmr == NULL)
        usb_config.reconnect_tmr = osTimerNew(furi_hal_usb_tmr_cb, osTimerOnce, NULL, NULL);
    usb_config.mode_changing = true;
    usb_if_next = usb_if_cur;
    osTimerStart(usb_config.reconnect_tmr, USB_RECONNECT_DELAY);
}

FuriHalUsbInterface* furi_hal_usb_get_config() {
    return usb_if_cur;
}

void furi_hal_usb_disable() {
    if(usb_config.enabled) {
        susp_evt(&udev, 0, 0);
        usbd_connect(&udev, false);
        usb_config.enabled = false;
        FURI_LOG_I(TAG, "USB Disable");
    }
}

void furi_hal_usb_enable() {
    if((!usb_config.enabled) && (usb_if_cur != NULL)) {
        usbd_connect(&udev, true);
        usb_config.enabled = true;
        FURI_LOG_I(TAG, "USB Enable");
    }
}

static void furi_hal_usb_tmr_cb(void* context) {
    furi_hal_usb_set_config(usb_if_next);
}

/* Get device / configuration descriptors */
static usbd_respond usb_descriptor_get(usbd_ctlreq* req, void** address, uint16_t* length) {
    const uint8_t dtype = req->wValue >> 8;
    const uint8_t dnumber = req->wValue & 0xFF;
    const void* desc;
    uint16_t len = 0;
    if(usb_if_cur == NULL) return usbd_fail;

    switch(dtype) {
    case USB_DTYPE_DEVICE:
        if(callback != NULL) {
            callback(FuriHalUsbStateEventDescriptorRequest, cb_ctx);
        }
        desc = usb_if_cur->dev_descr;
        break;
    case USB_DTYPE_CONFIGURATION:
        desc = usb_if_cur->cfg_descr;
        len = ((struct usb_string_descriptor*)(usb_if_cur->cfg_descr))->wString[0];
        break;
    case USB_DTYPE_STRING:
        if(dnumber == UsbDevLang) {
            desc = &dev_lang_desc;
        } else if((dnumber == UsbDevManuf) && (usb_if_cur->str_manuf_descr != NULL)) {
            desc = usb_if_cur->str_manuf_descr;
        } else if((dnumber == UsbDevProduct) && (usb_if_cur->str_prod_descr != NULL)) {
            desc = usb_if_cur->str_prod_descr;
        } else if((dnumber == UsbDevSerial) && (usb_if_cur->str_serial_descr != NULL)) {
            desc = usb_if_cur->str_serial_descr;
        } else
            return usbd_fail;
        break;
    default:
        return usbd_fail;
    }
    if(desc == NULL) return usbd_fail;

    if(len == 0) {
        len = ((struct usb_header_descriptor*)desc)->bLength;
    }
    *address = (void*)desc;
    *length = len;
    return usbd_ack;
}

void furi_hal_usb_set_state_callback(FuriHalUsbStateCallback cb, void* ctx) {
    callback = cb;
    cb_ctx = ctx;
}

static void reset_evt(usbd_device* dev, uint8_t event, uint8_t ep) {
    if(callback != NULL) {
        callback(FuriHalUsbStateEventReset, cb_ctx);
    }
}

static void susp_evt(usbd_device* dev, uint8_t event, uint8_t ep) {
    if((usb_if_cur != NULL) && (usb_config.connected == true)) {
        usb_config.connected = false;
        usb_if_cur->suspend(&udev);
    }
    if(callback != NULL) {
        callback(FuriHalUsbStateEventSuspend, cb_ctx);
    }
}

static void wkup_evt(usbd_device* dev, uint8_t event, uint8_t ep) {
    if((usb_if_cur != NULL) && (usb_config.connected == false)) {
        usb_config.connected = true;
        usb_if_cur->wakeup(&udev);
    }
    if(callback != NULL) {
        callback(FuriHalUsbStateEventWakeup, cb_ctx);
    }
}
