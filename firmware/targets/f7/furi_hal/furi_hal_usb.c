#include "furi_hal_version.h"
#include "furi_hal_usb_i.h"
#include "furi_hal_usb.h"
#include <furi_hal_power.h>
#include <stm32wbxx_ll_pwr.h>
#include <furi.h>

#include "usb.h"

#define TAG "FuriHalUsb"

#define USB_RECONNECT_DELAY 500

typedef struct {
    FuriThread* thread;
    bool enabled;
    bool connected;
    bool mode_lock;
    FuriHalUsbInterface* if_cur;
    FuriHalUsbInterface* if_next;
    void* if_ctx;
    FuriHalUsbStateCallback callback;
    void* cb_ctx;
} UsbSrv;

typedef enum {
    EventModeChange = (1 << 0),
    EventEnable = (1 << 1),
    EventDisable = (1 << 2),
    EventReinit = (1 << 3),

    EventReset = (1 << 4),
    EventRequest = (1 << 5),

    EventModeChangeStart = (1 << 6),
} UsbEvent;

#define USB_SRV_ALL_EVENTS                                                                    \
    (EventModeChange | EventEnable | EventDisable | EventReinit | EventReset | EventRequest | \
     EventModeChangeStart)

static UsbSrv usb;

static const struct usb_string_descriptor dev_lang_desc = USB_ARRAY_DESC(USB_LANGID_ENG_US);

static uint32_t ubuf[0x20];
usbd_device udev;

static int32_t furi_hal_usb_thread(void* context);
static usbd_respond usb_descriptor_get(usbd_ctlreq* req, void** address, uint16_t* length);
static void reset_evt(usbd_device* dev, uint8_t event, uint8_t ep);
static void susp_evt(usbd_device* dev, uint8_t event, uint8_t ep);
static void wkup_evt(usbd_device* dev, uint8_t event, uint8_t ep);

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

    usb.enabled = false;
    usb.if_cur = NULL;
    NVIC_SetPriority(USB_LP_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_SetPriority(USB_HP_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_EnableIRQ(USB_LP_IRQn);
    NVIC_EnableIRQ(USB_HP_IRQn);

    usb.thread = furi_thread_alloc();
    furi_thread_set_name(usb.thread, "UsbDriver");
    furi_thread_set_stack_size(usb.thread, 1024);
    furi_thread_set_callback(usb.thread, furi_hal_usb_thread);
    furi_thread_start(usb.thread);

    FURI_LOG_I(TAG, "Init OK");
}

bool furi_hal_usb_set_config(FuriHalUsbInterface* new_if, void* ctx) {
    if(usb.mode_lock) {
        return false;
    }

    usb.if_next = new_if;
    usb.if_ctx = ctx;
    if(usb.thread == NULL) {
        // Service thread hasn't started yet, so just save interface mode
        return true;
    }
    furi_assert(usb.thread);
    furi_thread_flags_set(furi_thread_get_id(usb.thread), EventModeChange);
    return true;
}

FuriHalUsbInterface* furi_hal_usb_get_config() {
    return usb.if_cur;
}

void furi_hal_usb_lock() {
    FURI_LOG_I(TAG, "Mode lock");
    usb.mode_lock = true;
}

void furi_hal_usb_unlock() {
    FURI_LOG_I(TAG, "Mode unlock");
    usb.mode_lock = false;
}

bool furi_hal_usb_is_locked() {
    return usb.mode_lock;
}

void furi_hal_usb_disable() {
    furi_assert(usb.thread);
    furi_thread_flags_set(furi_thread_get_id(usb.thread), EventDisable);
}

void furi_hal_usb_enable() {
    furi_assert(usb.thread);
    furi_thread_flags_set(furi_thread_get_id(usb.thread), EventEnable);
}

void furi_hal_usb_reinit() {
    furi_assert(usb.thread);
    furi_thread_flags_set(furi_thread_get_id(usb.thread), EventReinit);
}

/* Get device / configuration descriptors */
static usbd_respond usb_descriptor_get(usbd_ctlreq* req, void** address, uint16_t* length) {
    const uint8_t dtype = req->wValue >> 8;
    const uint8_t dnumber = req->wValue & 0xFF;
    const void* desc;
    uint16_t len = 0;
    if(usb.if_cur == NULL) return usbd_fail;

    switch(dtype) {
    case USB_DTYPE_DEVICE:
        furi_thread_flags_set(furi_thread_get_id(usb.thread), EventRequest);
        if(usb.callback != NULL) {
            usb.callback(FuriHalUsbStateEventDescriptorRequest, usb.cb_ctx);
        }
        desc = usb.if_cur->dev_descr;
        break;
    case USB_DTYPE_CONFIGURATION:
        desc = usb.if_cur->cfg_descr;
        len = ((struct usb_string_descriptor*)(usb.if_cur->cfg_descr))->wString[0];
        break;
    case USB_DTYPE_STRING:
        if(dnumber == UsbDevLang) {
            desc = &dev_lang_desc;
        } else if((dnumber == UsbDevManuf) && (usb.if_cur->str_manuf_descr != NULL)) {
            desc = usb.if_cur->str_manuf_descr;
        } else if((dnumber == UsbDevProduct) && (usb.if_cur->str_prod_descr != NULL)) {
            desc = usb.if_cur->str_prod_descr;
        } else if((dnumber == UsbDevSerial) && (usb.if_cur->str_serial_descr != NULL)) {
            desc = usb.if_cur->str_serial_descr;
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
    usb.callback = cb;
    usb.cb_ctx = ctx;
}

static void reset_evt(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
    furi_thread_flags_set(furi_thread_get_id(usb.thread), EventReset);
    if(usb.callback != NULL) {
        usb.callback(FuriHalUsbStateEventReset, usb.cb_ctx);
    }
}

static void susp_evt(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
    if((usb.if_cur != NULL) && (usb.connected == true)) {
        usb.connected = false;
        usb.if_cur->suspend(&udev);

        furi_hal_power_insomnia_exit();
    }
    if(usb.callback != NULL) {
        usb.callback(FuriHalUsbStateEventSuspend, usb.cb_ctx);
    }
}

static void wkup_evt(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
    if((usb.if_cur != NULL) && (usb.connected == false)) {
        usb.connected = true;
        usb.if_cur->wakeup(&udev);

        furi_hal_power_insomnia_enter();
    }
    if(usb.callback != NULL) {
        usb.callback(FuriHalUsbStateEventWakeup, usb.cb_ctx);
    }
}

static int32_t furi_hal_usb_thread(void* context) {
    UNUSED(context);
    bool usb_request_pending = false;
    uint8_t usb_wait_time = 0;
    FuriHalUsbInterface* if_new = NULL;
    FuriHalUsbInterface* if_ctx_new = NULL;

    if(usb.if_next != NULL) {
        furi_thread_flags_set(furi_thread_get_id(usb.thread), EventModeChange);
    }

    while(true) {
        uint32_t flags = furi_thread_flags_wait(USB_SRV_ALL_EVENTS, FuriFlagWaitAny, 500);
        if((flags & FuriFlagError) == 0) {
            if(flags & EventModeChange) {
                if(usb.if_next != usb.if_cur) {
                    if_new = usb.if_next;
                    if_ctx_new = usb.if_ctx;
                    if(usb.enabled) { // Disable current interface
                        susp_evt(&udev, 0, 0);
                        usbd_connect(&udev, false);
                        usb.enabled = false;
                        furi_delay_ms(USB_RECONNECT_DELAY);
                    }
                    flags |= EventModeChangeStart;
                }
            }
            if(flags & EventReinit) {
                // Temporary disable callback to avoid getting false reset events
                usbd_reg_event(&udev, usbd_evt_reset, NULL);
                FURI_LOG_I(TAG, "USB Reinit");
                susp_evt(&udev, 0, 0);
                usbd_connect(&udev, false);
                usb.enabled = false;

                usbd_enable(&udev, false);
                usbd_enable(&udev, true);

                if_new = usb.if_cur;
                furi_delay_ms(USB_RECONNECT_DELAY);
                flags |= EventModeChangeStart;
            }
            if(flags & EventModeChangeStart) { // Second stage of mode change process
                if(usb.if_cur != NULL) {
                    usb.if_cur->deinit(&udev);
                }
                if(if_new != NULL) {
                    if_new->init(&udev, if_new, if_ctx_new);
                    usbd_reg_event(&udev, usbd_evt_reset, reset_evt);
                    FURI_LOG_I(TAG, "USB Mode change done");
                    usb.enabled = true;
                }
                usb.if_cur = if_new;
            }
            if(flags & EventEnable) {
                if((!usb.enabled) && (usb.if_cur != NULL)) {
                    usbd_connect(&udev, true);
                    usb.enabled = true;
                    FURI_LOG_I(TAG, "USB Enable");
                }
            }
            if(flags & EventDisable) {
                if(usb.enabled) {
                    susp_evt(&udev, 0, 0);
                    usbd_connect(&udev, false);
                    usb.enabled = false;
                    usb_request_pending = false;
                    FURI_LOG_I(TAG, "USB Disable");
                }
            }
            if(flags & EventReset) {
                if(usb.enabled) {
                    usb_request_pending = true;
                    usb_wait_time = 0;
                }
            }
            if(flags & EventRequest) {
                usb_request_pending = false;
            }
        } else if(usb_request_pending) {
            usb_wait_time++;
            if(usb_wait_time > 4) {
                furi_hal_usb_reinit();
                usb_request_pending = false;
            }
        }
    }
    return 0;
}
