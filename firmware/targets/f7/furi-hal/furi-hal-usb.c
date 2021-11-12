#include "furi-hal-version.h"
#include "furi-hal-usb_i.h"
#include "furi-hal-usb.h"
#include <furi.h>

#include "usb.h"

#define TAG "FuriHalUsb"

#define USB_RECONNECT_DELAY 500

extern struct UsbInterface usb_cdc_single;
extern struct UsbInterface usb_cdc_dual;
extern struct UsbInterface usb_hid;

static struct UsbInterface* const usb_if_modes[UsbModesNum] = {
    NULL,
    &usb_cdc_single,
    &usb_cdc_dual,
    &usb_hid,
    NULL,//&usb_hid_u2f,
};

static const struct usb_string_descriptor dev_lang_desc = USB_ARRAY_DESC(USB_LANGID_ENG_US);

static uint32_t ubuf[0x20];
usbd_device udev;

static usbd_respond usb_descriptor_get (usbd_ctlreq *req, void **address, uint16_t *length);
static void susp_evt(usbd_device *dev, uint8_t event, uint8_t ep);
static void wkup_evt(usbd_device *dev, uint8_t event, uint8_t ep);

struct UsbCfg{
    osTimerId_t reconnect_tmr;
    UsbMode mode_cur;
    UsbMode mode_next;
    bool enabled;
    bool connected;
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

    usb_config.enabled = false;
    usb_config.reconnect_tmr = NULL;
    HAL_NVIC_SetPriority(USB_LP_IRQn, 5, 0);
    NVIC_EnableIRQ(USB_LP_IRQn);

    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_usb_set_config(UsbMode new_mode) {
    if (new_mode != usb_config.mode_cur) {
        if (usb_config.enabled) {
            usb_config.mode_next = new_mode;
            if (usb_config.reconnect_tmr == NULL)
                usb_config.reconnect_tmr = osTimerNew(furi_hal_usb_tmr_cb, osTimerOnce, NULL, NULL);
            furi_hal_usb_disable();
            osTimerStart(usb_config.reconnect_tmr, USB_RECONNECT_DELAY);
        }
        else {
            if (usb_if_modes[usb_config.mode_cur] != NULL)
                usb_if_modes[usb_config.mode_cur]->deinit(&udev);
            if (usb_if_modes[new_mode] != NULL) {
                usb_if_modes[new_mode]->init(&udev, usb_if_modes[new_mode]);
                FURI_LOG_I(TAG, "USB mode change %u -> %u", usb_config.mode_cur, new_mode);
                usb_config.enabled = true;
                usb_config.mode_cur = new_mode;
            }
        }
    }
}

UsbMode furi_hal_usb_get_config() {
    return usb_config.mode_cur;
}

void furi_hal_usb_disable() {
    if (usb_config.enabled) {
        susp_evt(&udev, 0, 0);
        usbd_connect(&udev, false);
        usb_config.enabled = false;
        FURI_LOG_I(TAG, "USB Disable");
    }
}

void furi_hal_usb_enable() {
    if ((!usb_config.enabled) && (usb_if_modes[usb_config.mode_cur] != NULL)) {
        usbd_connect(&udev, true);
        usb_config.enabled = true;
        FURI_LOG_I(TAG, "USB Enable");
    }
}

static void furi_hal_usb_tmr_cb(void* context) {
    furi_hal_usb_set_config(usb_config.mode_next);
}

/* Get device / configuration descriptors */
static usbd_respond usb_descriptor_get (usbd_ctlreq *req, void **address, uint16_t *length) {
    const uint8_t dtype = req->wValue >> 8;
    const uint8_t dnumber = req->wValue & 0xFF;
    const void* desc;
    uint16_t len = 0;
    if (usb_if_modes[usb_config.mode_cur] == NULL)
        return usbd_fail;

    switch (dtype) {
    case USB_DTYPE_DEVICE:
        desc = usb_if_modes[usb_config.mode_cur]->dev_descr;
        break;
    case USB_DTYPE_CONFIGURATION:
        desc = usb_if_modes[usb_config.mode_cur]->cfg_descr;
        len = ((struct usb_string_descriptor*)(usb_if_modes[usb_config.mode_cur]->cfg_descr))->wString[0];
        break;
    case USB_DTYPE_STRING:
        if (dnumber == UsbDevLang) {
            desc = &dev_lang_desc;
        } else if (dnumber == UsbDevManuf) {
            desc = usb_if_modes[usb_config.mode_cur]->str_manuf_descr;
        } else if (dnumber == UsbDevProduct) {
            desc = usb_if_modes[usb_config.mode_cur]->str_prod_descr;
        } else if (dnumber == UsbDevSerial) {
            desc = usb_if_modes[usb_config.mode_cur]->str_serial_descr;
        } else 
            return usbd_fail;
        break;
    default:
        return usbd_fail;
    }
    if (desc == NULL)
        return usbd_fail;

    if (len == 0) {
        len = ((struct usb_header_descriptor*)desc)->bLength;
    }
    *address = (void*)desc;
    *length = len;
    return usbd_ack;
}

static void susp_evt(usbd_device *dev, uint8_t event, uint8_t ep) {
    if ((usb_if_modes[usb_config.mode_cur] != NULL) && (usb_config.connected == true)) {
        usb_config.connected = false;
        usb_if_modes[usb_config.mode_cur]->suspend(&udev);
    }
}

static void wkup_evt(usbd_device *dev, uint8_t event, uint8_t ep) {
    if ((usb_if_modes[usb_config.mode_cur] != NULL) && (usb_config.connected == false)) {
        usb_config.connected = true;
        usb_if_modes[usb_config.mode_cur]->wakeup(&udev);
    } 
}
