#include <furi_hal_version.h>
#include <furi_hal_usb_i.h>
#include <furi_hal_usb.h>
#include <furi_hal_power.h>

#include <stm32wbxx_ll_pwr.h>
#include <stm32wbxx_ll_rcc.h>
#include <furi.h>
#include <toolbox/api_lock.h>

#include "usb.h"

#define TAG "FuriHalUsb"

#define USB_RECONNECT_DELAY 500

typedef enum {
    UsbApiEventTypeSetConfig,
    UsbApiEventTypeGetConfig,
    UsbApiEventTypeLock,
    UsbApiEventTypeUnlock,
    UsbApiEventTypeIsLocked,
    UsbApiEventTypeEnable,
    UsbApiEventTypeDisable,
    UsbApiEventTypeReinit,
    UsbApiEventTypeSetStateCallback,
} UsbApiEventType;

typedef struct {
    FuriHalUsbStateCallback callback;
    void* context;
} UsbApiEventDataStateCallback;

typedef struct {
    FuriHalUsbInterface* interface;
    void* context;
} UsbApiEventDataInterface;

typedef union {
    UsbApiEventDataStateCallback state_callback;
    UsbApiEventDataInterface interface;
} UsbApiEventData;

typedef union {
    bool bool_value;
    void* void_value;
} UsbApiEventReturnData;

typedef struct {
    FuriApiLock lock;
    UsbApiEventType type;
    UsbApiEventData data;
    UsbApiEventReturnData* return_data;
} UsbApiEventMessage;

typedef struct {
    FuriThread* thread;
    FuriMessageQueue* queue;
    bool enabled;
    bool connected;
    bool mode_lock;
    bool request_pending;
    FuriHalUsbInterface* interface;
    void* interface_context;
    FuriHalUsbStateCallback callback;
    void* callback_context;
} UsbSrv;

typedef enum {
    UsbEventReset = (1 << 0),
    UsbEventRequest = (1 << 1),
    UsbEventMessage = (1 << 2),
} UsbEvent;

#define USB_SRV_ALL_EVENTS (UsbEventReset | UsbEventRequest | UsbEventMessage)

PLACE_IN_SECTION("MB_MEM2") static UsbSrv usb = {0};
PLACE_IN_SECTION("MB_MEM2") static uint32_t ubuf[0x20];
PLACE_IN_SECTION("MB_MEM2") usbd_device udev;

static const struct usb_string_descriptor dev_lang_desc = USB_ARRAY_DESC(USB_LANGID_ENG_US);

static int32_t furi_hal_usb_thread(void* context);
static usbd_respond usb_descriptor_get(usbd_ctlreq* req, void** address, uint16_t* length);
static void reset_evt(usbd_device* dev, uint8_t event, uint8_t ep);
static void susp_evt(usbd_device* dev, uint8_t event, uint8_t ep);
static void wkup_evt(usbd_device* dev, uint8_t event, uint8_t ep);

/* Low-level init */
void furi_hal_usb_init(void) {
    LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_PLLSAI1);

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

    FURI_CRITICAL_ENTER();
    usbd_enable(&udev, true);
    FURI_CRITICAL_EXIT();

    usbd_reg_descr(&udev, usb_descriptor_get);
    usbd_reg_event(&udev, usbd_evt_susp, susp_evt);
    usbd_reg_event(&udev, usbd_evt_wkup, wkup_evt);
    // Reset callback will be enabled after first mode change to avoid getting false reset events

    usb.enabled = false;
    usb.interface = NULL;
    NVIC_SetPriority(USB_LP_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
    NVIC_SetPriority(USB_HP_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_EnableIRQ(USB_LP_IRQn);
    NVIC_EnableIRQ(USB_HP_IRQn);

    usb.queue = furi_message_queue_alloc(1, sizeof(UsbApiEventMessage));
    usb.thread = furi_thread_alloc_service("UsbDriver", 1024, furi_hal_usb_thread, NULL);
    furi_thread_start(usb.thread);

    FURI_LOG_I(TAG, "Init OK");
}

static void furi_hal_usb_send_message(UsbApiEventMessage* message) {
    furi_message_queue_put(usb.queue, message, FuriWaitForever);
    furi_thread_flags_set(furi_thread_get_id(usb.thread), UsbEventMessage);
    api_lock_wait_unlock_and_free(message->lock);
}

bool furi_hal_usb_set_config(FuriHalUsbInterface* new_if, void* ctx) {
    UsbApiEventReturnData return_data = {
        .bool_value = false,
    };

    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = UsbApiEventTypeSetConfig,
        .data.interface =
            {
                .interface = new_if,
                .context = ctx,
            },
        .return_data = &return_data,
    };

    furi_hal_usb_send_message(&msg);
    return return_data.bool_value;
}

FuriHalUsbInterface* furi_hal_usb_get_config(void) {
    UsbApiEventReturnData return_data = {
        .void_value = NULL,
    };

    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = UsbApiEventTypeGetConfig,
        .return_data = &return_data,
    };

    furi_hal_usb_send_message(&msg);
    return return_data.void_value;
}

void furi_hal_usb_lock(void) {
    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = UsbApiEventTypeLock,
    };

    furi_hal_usb_send_message(&msg);
}

void furi_hal_usb_unlock(void) {
    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = UsbApiEventTypeUnlock,
    };

    furi_hal_usb_send_message(&msg);
}

bool furi_hal_usb_is_locked(void) {
    UsbApiEventReturnData return_data = {
        .bool_value = false,
    };

    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = UsbApiEventTypeIsLocked,
        .return_data = &return_data,
    };

    furi_hal_usb_send_message(&msg);
    return return_data.bool_value;
}

void furi_hal_usb_disable(void) {
    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = UsbApiEventTypeDisable,
    };

    furi_hal_usb_send_message(&msg);
}

void furi_hal_usb_enable(void) {
    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = UsbApiEventTypeEnable,
    };

    furi_hal_usb_send_message(&msg);
}

void furi_hal_usb_reinit(void) {
    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = UsbApiEventTypeReinit,
    };

    furi_hal_usb_send_message(&msg);
}

void furi_hal_usb_set_state_callback(FuriHalUsbStateCallback cb, void* ctx) {
    UsbApiEventMessage msg = {
        .lock = api_lock_alloc_locked(),
        .type = UsbApiEventTypeSetStateCallback,
        .data.state_callback =
            {
                .callback = cb,
                .context = ctx,
            },
    };

    furi_hal_usb_send_message(&msg);
}

/* Get device / configuration descriptors */
static usbd_respond usb_descriptor_get(usbd_ctlreq* req, void** address, uint16_t* length) {
    const uint8_t dtype = req->wValue >> 8;
    const uint8_t dnumber = req->wValue & 0xFF;
    const void* desc;
    uint16_t len = 0;
    if(usb.interface == NULL) return usbd_fail;

    switch(dtype) {
    case USB_DTYPE_DEVICE:
        furi_thread_flags_set(furi_thread_get_id(usb.thread), UsbEventRequest);
        if(usb.callback != NULL) {
            usb.callback(FuriHalUsbStateEventDescriptorRequest, usb.callback_context);
        }
        desc = usb.interface->dev_descr;
        break;
    case USB_DTYPE_CONFIGURATION:
        desc = usb.interface->cfg_descr;
        len = ((struct usb_string_descriptor*)(usb.interface->cfg_descr))->wString[0];
        break;
    case USB_DTYPE_STRING:
        if(dnumber == UsbDevLang) {
            desc = &dev_lang_desc;
        } else if((dnumber == UsbDevManuf) && (usb.interface->str_manuf_descr != NULL)) {
            desc = usb.interface->str_manuf_descr;
        } else if((dnumber == UsbDevProduct) && (usb.interface->str_prod_descr != NULL)) {
            desc = usb.interface->str_prod_descr;
        } else if((dnumber == UsbDevSerial) && (usb.interface->str_serial_descr != NULL)) {
            desc = usb.interface->str_serial_descr;
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

static void reset_evt(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
    furi_thread_flags_set(furi_thread_get_id(usb.thread), UsbEventReset);
    if(usb.callback != NULL) {
        usb.callback(FuriHalUsbStateEventReset, usb.callback_context);
    }
}

static void susp_evt(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
    if((usb.interface != NULL) && (usb.connected == true)) {
        usb.connected = false;
        usb.interface->suspend(&udev);

        furi_hal_power_insomnia_exit();
    }
    if(usb.callback != NULL) {
        usb.callback(FuriHalUsbStateEventSuspend, usb.callback_context);
    }
}

static void wkup_evt(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
    if((usb.interface != NULL) && (usb.connected == false)) {
        usb.connected = true;
        usb.interface->wakeup(&udev);

        furi_hal_power_insomnia_enter();
    }
    if(usb.callback != NULL) {
        usb.callback(FuriHalUsbStateEventWakeup, usb.callback_context);
    }
}

static void usb_process_mode_start(FuriHalUsbInterface* interface, void* context) {
    if(usb.interface != NULL) {
        usb.interface->deinit(&udev);
    }

    __disable_irq();
    usb.interface = interface;
    usb.interface_context = context;
    __enable_irq();

    if(interface != NULL) {
        interface->init(&udev, interface, context);
        usbd_reg_event(&udev, usbd_evt_reset, reset_evt);
        FURI_LOG_I(TAG, "USB Mode change done");
        usb.enabled = true;
    }
}

static void usb_process_mode_change(FuriHalUsbInterface* interface, void* context) {
    if((interface != usb.interface) || (context != usb.interface_context)) {
        if(usb.enabled) {
            // Disable current interface
            susp_evt(&udev, 0, 0);
            usbd_connect(&udev, false);
            usb.enabled = false;
            furi_delay_ms(USB_RECONNECT_DELAY);
        }
        usb_process_mode_start(interface, context);
    }
}

static void usb_process_mode_reinit(void) {
    // Temporary disable callback to avoid getting false reset events
    usbd_reg_event(&udev, usbd_evt_reset, NULL);
    FURI_LOG_I(TAG, "USB Reinit");
    susp_evt(&udev, 0, 0);
    usbd_connect(&udev, false);
    usb.enabled = false;

    FURI_CRITICAL_ENTER();
    usbd_enable(&udev, false);
    usbd_enable(&udev, true);
    FURI_CRITICAL_EXIT();

    furi_delay_ms(USB_RECONNECT_DELAY);
    usb_process_mode_start(usb.interface, usb.interface_context);
}

static bool usb_process_set_config(FuriHalUsbInterface* interface, void* context) {
    if(usb.mode_lock) {
        return false;
    } else {
        usb_process_mode_change(interface, context);
        return true;
    }
}

static void usb_process_enable(bool enable) {
    if(enable) {
        if((!usb.enabled) && (usb.interface != NULL)) {
            usbd_connect(&udev, true);
            usb.enabled = true;
            FURI_LOG_I(TAG, "USB Enable");
        }
    } else {
        if(usb.enabled) {
            susp_evt(&udev, 0, 0);
            usbd_connect(&udev, false);
            usb.enabled = false;
            usb.request_pending = false;
            FURI_LOG_I(TAG, "USB Disable");
        }
    }
}

static void usb_process_message(UsbApiEventMessage* message) {
    switch(message->type) {
    case UsbApiEventTypeSetConfig:
        message->return_data->bool_value = usb_process_set_config(
            message->data.interface.interface, message->data.interface.context);
        break;
    case UsbApiEventTypeGetConfig:
        message->return_data->void_value = usb.interface;
        break;
    case UsbApiEventTypeLock:
        FURI_LOG_I(TAG, "Mode lock");
        usb.mode_lock = true;
        break;
    case UsbApiEventTypeUnlock:
        FURI_LOG_I(TAG, "Mode unlock");
        usb.mode_lock = false;
        break;
    case UsbApiEventTypeIsLocked:
        message->return_data->bool_value = usb.mode_lock;
        break;
    case UsbApiEventTypeDisable:
        usb_process_enable(false);
        break;
    case UsbApiEventTypeEnable:
        usb_process_enable(true);
        break;
    case UsbApiEventTypeReinit:
        usb_process_mode_reinit();
        break;
    case UsbApiEventTypeSetStateCallback:
        usb.callback = message->data.state_callback.callback;
        usb.callback_context = message->data.state_callback.context;
        break;
    }

    api_lock_unlock(message->lock);
}

static int32_t furi_hal_usb_thread(void* context) {
    UNUSED(context);
    uint8_t usb_wait_time = 0;

    if(furi_message_queue_get_count(usb.queue) > 0) {
        furi_thread_flags_set(furi_thread_get_id(usb.thread), UsbEventMessage);
    }

    while(true) {
        uint32_t flags = furi_thread_flags_wait(USB_SRV_ALL_EVENTS, FuriFlagWaitAny, 500);

        {
            UsbApiEventMessage message;
            if(furi_message_queue_get(usb.queue, &message, 0) == FuriStatusOk) {
                usb_process_message(&message);
            }
        }

        if((flags & FuriFlagError) == 0) {
            if(flags & UsbEventReset) {
                if(usb.enabled) {
                    usb.request_pending = true;
                    usb_wait_time = 0;
                }
            }
            if(flags & UsbEventRequest) {
                usb.request_pending = false;
            }
        } else if(usb.request_pending) {
            usb_wait_time++;
            if(usb_wait_time > 4) {
                usb_process_mode_reinit();
                usb.request_pending = false;
            }
        }
    }
    return 0;
}
