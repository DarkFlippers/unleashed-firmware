#include <furi_hal_version.h>
#include <furi_hal_usb_i.h>
#include <furi_hal_usb.h>
#include <furi_hal_usb_hid.h>
#include <furi.h>

#include "usb.h"
#include "usb_hid.h"

#define HID_EP_IN 0x81
#define HID_EP_SZ 0x10

#define HID_INTERVAL 2

#define HID_VID_DEFAULT 0x046D
#define HID_PID_DEFAULT 0xC529

struct HidIntfDescriptor {
    struct usb_interface_descriptor hid;
    struct usb_hid_descriptor hid_desc;
    struct usb_endpoint_descriptor hid_ep_in;
};

struct HidConfigDescriptor {
    struct usb_config_descriptor config;
    struct HidIntfDescriptor intf_0;
} FURI_PACKED;

enum HidReportId {
    ReportIdKeyboard = 1,
    ReportIdMouse = 2,
    ReportIdConsumer = 3,
};

/* HID report descriptor: keyboard + mouse + consumer control */
static const uint8_t hid_report_desc[] = {
    // clang-format off
    HID_USAGE_PAGE(HID_PAGE_DESKTOP),
    HID_USAGE(HID_DESKTOP_KEYBOARD),
    HID_COLLECTION(HID_APPLICATION_COLLECTION),
        HID_REPORT_ID(ReportIdKeyboard), 
        // Keyboard report
        HID_USAGE_PAGE(HID_DESKTOP_KEYPAD),
        HID_USAGE_MINIMUM(HID_KEYBOARD_L_CTRL),
        HID_USAGE_MAXIMUM(HID_KEYBOARD_R_GUI),
        HID_LOGICAL_MINIMUM(0),
        HID_LOGICAL_MAXIMUM(1),
        HID_REPORT_SIZE(1),
        HID_REPORT_COUNT(8),
        // Input - Modifier keys byte
        HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
        
        HID_REPORT_COUNT(1),
        HID_REPORT_SIZE(8),
        // Input - Reserved byte
        HID_INPUT(HID_IOF_CONSTANT | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

        HID_USAGE_PAGE(HID_PAGE_LED),
        HID_REPORT_COUNT(8),
        HID_REPORT_SIZE(1),
        HID_USAGE_MINIMUM(1),
        HID_USAGE_MAXIMUM(8),
        // Output - LEDs
        HID_OUTPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

        HID_REPORT_COUNT(HID_KB_MAX_KEYS),
        HID_REPORT_SIZE(8),
        HID_LOGICAL_MINIMUM(0),
        HID_LOGICAL_MAXIMUM(101),
        HID_USAGE_PAGE(HID_DESKTOP_KEYPAD),
        HID_USAGE_MINIMUM(0),
        HID_USAGE_MAXIMUM(101),
        // Input - Key codes
        HID_INPUT(HID_IOF_DATA | HID_IOF_ARRAY | HID_IOF_ABSOLUTE),
    HID_END_COLLECTION,

    HID_USAGE_PAGE(HID_PAGE_DESKTOP),
    HID_USAGE(HID_DESKTOP_MOUSE),
    HID_COLLECTION(HID_APPLICATION_COLLECTION),
        HID_USAGE(HID_DESKTOP_POINTER),
        HID_COLLECTION(HID_PHYSICAL_COLLECTION),
            HID_REPORT_ID(ReportIdMouse),
            // Mouse report
            HID_USAGE_PAGE(HID_PAGE_BUTTON),
            HID_USAGE_MINIMUM(1),
            HID_USAGE_MAXIMUM(3),
            HID_LOGICAL_MINIMUM(0),
            HID_LOGICAL_MAXIMUM(1),
            HID_REPORT_COUNT(3),
            HID_REPORT_SIZE(1),
            // Input - Mouse keys
            HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),

            HID_REPORT_SIZE(1),
            HID_REPORT_COUNT(5),
            // Input - Mouse keys padding
            HID_INPUT(HID_IOF_CONSTANT | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
            
            HID_USAGE_PAGE(HID_PAGE_DESKTOP),
            HID_USAGE(HID_DESKTOP_X),
            HID_USAGE(HID_DESKTOP_Y),
            HID_USAGE(HID_DESKTOP_WHEEL),
            HID_LOGICAL_MINIMUM(-127),
            HID_LOGICAL_MAXIMUM(127),
            HID_REPORT_SIZE(8),
            HID_REPORT_COUNT(3),
            // Input - Mouse movement data (x, y, scroll)
            HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_RELATIVE),
        HID_END_COLLECTION,
    HID_END_COLLECTION,

    HID_USAGE_PAGE(HID_PAGE_CONSUMER),
    HID_USAGE(HID_CONSUMER_CONTROL),
    HID_COLLECTION(HID_APPLICATION_COLLECTION),
        HID_REPORT_ID(ReportIdConsumer),
        // Consumer report
        HID_LOGICAL_MINIMUM(0),
        HID_RI_LOGICAL_MAXIMUM(16, 0x3FF),
        HID_USAGE_MINIMUM(0),
        HID_RI_USAGE_MAXIMUM(16, 0x3FF),
        HID_REPORT_COUNT(HID_CONSUMER_MAX_KEYS),
        HID_REPORT_SIZE(16),
        // Input - Consumer control keys
        HID_INPUT(HID_IOF_DATA | HID_IOF_ARRAY | HID_IOF_ABSOLUTE),
    HID_END_COLLECTION,
    // clang-format on
};

/* Device descriptor */
static struct usb_device_descriptor hid_device_desc = {
    .bLength = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DTYPE_DEVICE,
    .bcdUSB = VERSION_BCD(2, 0, 0),
    .bDeviceClass = USB_CLASS_PER_INTERFACE,
    .bDeviceSubClass = USB_SUBCLASS_NONE,
    .bDeviceProtocol = USB_PROTO_NONE,
    .bMaxPacketSize0 = USB_EP0_SIZE,
    .idVendor = HID_VID_DEFAULT,
    .idProduct = HID_PID_DEFAULT,
    .bcdDevice = VERSION_BCD(1, 0, 0),
    .iManufacturer = 0,
    .iProduct = 0,
    .iSerialNumber = 0,
    .bNumConfigurations = 1,
};

/* Device configuration descriptor */
static const struct HidConfigDescriptor hid_cfg_desc = {
    .config =
        {
            .bLength = sizeof(struct usb_config_descriptor),
            .bDescriptorType = USB_DTYPE_CONFIGURATION,
            .wTotalLength = sizeof(struct HidConfigDescriptor),
            .bNumInterfaces = 1,
            .bConfigurationValue = 1,
            .iConfiguration = NO_DESCRIPTOR,
            .bmAttributes = USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPOWERED,
            .bMaxPower = USB_CFG_POWER_MA(500),
        },
    .intf_0 =
        {
            .hid =
                {
                    .bLength = sizeof(struct usb_interface_descriptor),
                    .bDescriptorType = USB_DTYPE_INTERFACE,
                    .bInterfaceNumber = 0,
                    .bAlternateSetting = 0,
                    .bNumEndpoints = 1,
                    .bInterfaceClass = USB_CLASS_HID,
                    .bInterfaceSubClass = USB_HID_SUBCLASS_BOOT,
                    .bInterfaceProtocol = USB_HID_PROTO_KEYBOARD,
                    .iInterface = NO_DESCRIPTOR,
                },
            .hid_desc =
                {
                    .bLength = sizeof(struct usb_hid_descriptor),
                    .bDescriptorType = USB_DTYPE_HID,
                    .bcdHID = VERSION_BCD(1, 0, 0),
                    .bCountryCode = USB_HID_COUNTRY_NONE,
                    .bNumDescriptors = 1,
                    .bDescriptorType0 = USB_DTYPE_HID_REPORT,
                    .wDescriptorLength0 = sizeof(hid_report_desc),
                },
            .hid_ep_in =
                {
                    .bLength = sizeof(struct usb_endpoint_descriptor),
                    .bDescriptorType = USB_DTYPE_ENDPOINT,
                    .bEndpointAddress = HID_EP_IN,
                    .bmAttributes = USB_EPTYPE_INTERRUPT,
                    .wMaxPacketSize = HID_EP_SZ,
                    .bInterval = HID_INTERVAL,
                },
        },
};

struct HidReportMouse {
    uint8_t report_id;
    uint8_t btn;
    int8_t x;
    int8_t y;
    int8_t wheel;
} FURI_PACKED;

struct HidReportKB {
    uint8_t report_id;
    struct {
        uint8_t mods;
        uint8_t reserved;
        uint8_t btn[HID_KB_MAX_KEYS];
    } boot;
} FURI_PACKED;

struct HidReportConsumer {
    uint8_t report_id;
    uint16_t btn[HID_CONSUMER_MAX_KEYS];
} FURI_PACKED;

struct HidReportLED {
    uint8_t report_id;
    uint8_t led_state;
} FURI_PACKED;

static struct HidReport {
    struct HidReportKB keyboard;
    struct HidReportMouse mouse;
    struct HidReportConsumer consumer;
} FURI_PACKED hid_report;

static void hid_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx);
static void hid_deinit(usbd_device* dev);
static void hid_on_wakeup(usbd_device* dev);
static void hid_on_suspend(usbd_device* dev);

FuriHalUsbInterface usb_hid = {
    .init = hid_init,
    .deinit = hid_deinit,
    .wakeup = hid_on_wakeup,
    .suspend = hid_on_suspend,

    .dev_descr = (struct usb_device_descriptor*)&hid_device_desc,

    .str_manuf_descr = NULL,
    .str_prod_descr = NULL,
    .str_serial_descr = NULL,

    .cfg_descr = (void*)&hid_cfg_desc,
};

static bool hid_send_report(uint8_t report_id);
static usbd_respond hid_ep_config(usbd_device* dev, uint8_t cfg);
static usbd_respond hid_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback);
static usbd_device* usb_dev;
static FuriSemaphore* hid_semaphore = NULL;
static bool hid_connected = false;
static HidStateCallback callback;
static void* cb_ctx;
static uint8_t led_state;
static bool boot_protocol = false;

bool furi_hal_hid_is_connected(void) {
    return hid_connected;
}

uint8_t furi_hal_hid_get_led_state(void) {
    return led_state;
}

void furi_hal_hid_set_state_callback(HidStateCallback cb, void* ctx) {
    if(callback != NULL) {
        if(hid_connected == true) callback(false, cb_ctx);
    }

    callback = cb;
    cb_ctx = ctx;

    if(callback != NULL) {
        if(hid_connected == true) callback(true, cb_ctx);
    }
}

bool furi_hal_hid_kb_press(uint16_t button) {
    for(uint8_t key_nb = 0; key_nb < HID_KB_MAX_KEYS; key_nb++) {
        if(hid_report.keyboard.boot.btn[key_nb] == 0) {
            hid_report.keyboard.boot.btn[key_nb] = button & 0xFF;
            break;
        }
    }
    hid_report.keyboard.boot.mods |= (button >> 8);
    return hid_send_report(ReportIdKeyboard);
}

bool furi_hal_hid_kb_release(uint16_t button) {
    for(uint8_t key_nb = 0; key_nb < HID_KB_MAX_KEYS; key_nb++) {
        if(hid_report.keyboard.boot.btn[key_nb] == (button & 0xFF)) {
            hid_report.keyboard.boot.btn[key_nb] = 0;
            break;
        }
    }
    hid_report.keyboard.boot.mods &= ~(button >> 8);
    return hid_send_report(ReportIdKeyboard);
}

bool furi_hal_hid_kb_release_all(void) {
    for(uint8_t key_nb = 0; key_nb < HID_KB_MAX_KEYS; key_nb++) {
        hid_report.keyboard.boot.btn[key_nb] = 0;
    }
    hid_report.keyboard.boot.mods = 0;
    return hid_send_report(ReportIdKeyboard);
}

bool furi_hal_hid_mouse_move(int8_t dx, int8_t dy) {
    hid_report.mouse.x = dx;
    hid_report.mouse.y = dy;
    bool state = hid_send_report(ReportIdMouse);
    hid_report.mouse.x = 0;
    hid_report.mouse.y = 0;
    return state;
}

bool furi_hal_hid_mouse_press(uint8_t button) {
    hid_report.mouse.btn |= button;
    return hid_send_report(ReportIdMouse);
}

bool furi_hal_hid_mouse_release(uint8_t button) {
    hid_report.mouse.btn &= ~button;
    return hid_send_report(ReportIdMouse);
}

bool furi_hal_hid_mouse_scroll(int8_t delta) {
    hid_report.mouse.wheel = delta;
    bool state = hid_send_report(ReportIdMouse);
    hid_report.mouse.wheel = 0;
    return state;
}

bool furi_hal_hid_consumer_key_press(uint16_t button) {
    for(uint8_t key_nb = 0; key_nb < HID_CONSUMER_MAX_KEYS; key_nb++) {
        if(hid_report.consumer.btn[key_nb] == 0) {
            hid_report.consumer.btn[key_nb] = button;
            break;
        }
    }
    return hid_send_report(ReportIdConsumer);
}

bool furi_hal_hid_consumer_key_release(uint16_t button) {
    for(uint8_t key_nb = 0; key_nb < HID_CONSUMER_MAX_KEYS; key_nb++) {
        if(hid_report.consumer.btn[key_nb] == button) {
            hid_report.consumer.btn[key_nb] = 0;
            break;
        }
    }
    return hid_send_report(ReportIdConsumer);
}

bool furi_hal_hid_consumer_key_release_all(void) {
    for(uint8_t key_nb = 0; key_nb < HID_CONSUMER_MAX_KEYS; key_nb++) {
        hid_report.consumer.btn[key_nb] = 0;
    }
    return hid_send_report(ReportIdConsumer);
}

static void* hid_set_string_descr(char* str) {
    furi_assert(str);

    size_t len = strlen(str);
    struct usb_string_descriptor* dev_str_desc = malloc(len * 2 + 2);
    dev_str_desc->bLength = len * 2 + 2;
    dev_str_desc->bDescriptorType = USB_DTYPE_STRING;
    for(size_t i = 0; i < len; i++)
        dev_str_desc->wString[i] = str[i];

    return dev_str_desc;
}

static void hid_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx) {
    UNUSED(intf);
    FuriHalUsbHidConfig* cfg = (FuriHalUsbHidConfig*)ctx;
    if(hid_semaphore == NULL) hid_semaphore = furi_semaphore_alloc(1, 1);
    usb_dev = dev;
    hid_report.keyboard.report_id = ReportIdKeyboard;
    hid_report.mouse.report_id = ReportIdMouse;
    hid_report.consumer.report_id = ReportIdConsumer;

    usb_hid.dev_descr->iManufacturer = 0;
    usb_hid.dev_descr->iProduct = 0;
    usb_hid.str_manuf_descr = NULL;
    usb_hid.str_prod_descr = NULL;
    usb_hid.dev_descr->idVendor = HID_VID_DEFAULT;
    usb_hid.dev_descr->idProduct = HID_PID_DEFAULT;

    if(cfg != NULL) {
        usb_hid.dev_descr->idVendor = cfg->vid;
        usb_hid.dev_descr->idProduct = cfg->pid;

        if(cfg->manuf[0] != '\0') {
            usb_hid.str_manuf_descr = hid_set_string_descr(cfg->manuf);
            usb_hid.dev_descr->iManufacturer = UsbDevManuf;
        }

        if(cfg->product[0] != '\0') {
            usb_hid.str_prod_descr = hid_set_string_descr(cfg->product);
            usb_hid.dev_descr->iProduct = UsbDevProduct;
        }
    }

    usbd_reg_config(dev, hid_ep_config);
    usbd_reg_control(dev, hid_control);

    usbd_connect(dev, true);
}

static void hid_deinit(usbd_device* dev) {
    usbd_reg_config(dev, NULL);
    usbd_reg_control(dev, NULL);

    free(usb_hid.str_manuf_descr);
    free(usb_hid.str_prod_descr);
}

static void hid_on_wakeup(usbd_device* dev) {
    UNUSED(dev);
    if(!hid_connected) {
        hid_connected = true;
        if(callback != NULL) {
            callback(true, cb_ctx);
        }
    }
}

static void hid_on_suspend(usbd_device* dev) {
    UNUSED(dev);
    if(hid_connected) {
        hid_connected = false;
        furi_semaphore_release(hid_semaphore);
        if(callback != NULL) {
            callback(false, cb_ctx);
        }
    }
}

static bool hid_send_report(uint8_t report_id) {
    if((hid_semaphore == NULL) || (hid_connected == false)) return false;
    if((boot_protocol == true) && (report_id != ReportIdKeyboard)) return false;

    FuriStatus status = furi_semaphore_acquire(hid_semaphore, HID_INTERVAL * 2);
    if(status == FuriStatusErrorTimeout) {
        return false;
    }
    furi_check(status == FuriStatusOk);
    if(hid_connected == false) {
        return false;
    }
    if(boot_protocol == true) {
        usbd_ep_write(
            usb_dev, HID_EP_IN, &hid_report.keyboard.boot, sizeof(hid_report.keyboard.boot));
    } else {
        if(report_id == ReportIdKeyboard)
            usbd_ep_write(usb_dev, HID_EP_IN, &hid_report.keyboard, sizeof(hid_report.keyboard));
        else if(report_id == ReportIdMouse)
            usbd_ep_write(usb_dev, HID_EP_IN, &hid_report.mouse, sizeof(hid_report.mouse));
        else if(report_id == ReportIdConsumer)
            usbd_ep_write(usb_dev, HID_EP_IN, &hid_report.consumer, sizeof(hid_report.consumer));
    }
    return true;
}

static void hid_txrx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    if(event == usbd_evt_eptx) {
        furi_semaphore_release(hid_semaphore);
    } else if(boot_protocol == true) {
        usbd_ep_read(usb_dev, ep, &led_state, sizeof(led_state));
    } else {
        struct HidReportLED leds;
        usbd_ep_read(usb_dev, ep, &leds, sizeof(leds));
        led_state = leds.led_state;
    }
}

/* Configure endpoints */
static usbd_respond hid_ep_config(usbd_device* dev, uint8_t cfg) {
    switch(cfg) {
    case 0:
        /* deconfiguring device */
        usbd_ep_deconfig(dev, HID_EP_IN);
        usbd_reg_endpoint(dev, HID_EP_IN, 0);
        return usbd_ack;
    case 1:
        /* configuring device */
        usbd_ep_config(dev, HID_EP_IN, USB_EPTYPE_INTERRUPT, HID_EP_SZ);
        usbd_reg_endpoint(dev, HID_EP_IN, hid_txrx_ep_callback);
        usbd_ep_write(dev, HID_EP_IN, 0, 0);
        boot_protocol = false; /* BIOS will SET_PROTOCOL if it wants this */
        return usbd_ack;
    default:
        return usbd_fail;
    }
}

/* Control requests handler */
static usbd_respond hid_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback) {
    UNUSED(callback);
    /* HID control requests */
    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
           (USB_REQ_INTERFACE | USB_REQ_CLASS) &&
       req->wIndex == 0) {
        switch(req->bRequest) {
        case USB_HID_SETIDLE:
            return usbd_ack;
        case USB_HID_GETREPORT:
            if(boot_protocol == true) {
                dev->status.data_ptr = &hid_report.keyboard.boot;
                dev->status.data_count = sizeof(hid_report.keyboard.boot);
            } else {
                dev->status.data_ptr = &hid_report;
                dev->status.data_count = sizeof(hid_report);
            }
            return usbd_ack;
        case USB_HID_SETPROTOCOL:
            if(req->wValue == 0)
                boot_protocol = true;
            else if(req->wValue == 1)
                boot_protocol = false;
            else
                return usbd_fail;
            return usbd_ack;
        default:
            return usbd_fail;
        }
    }
    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
           (USB_REQ_INTERFACE | USB_REQ_STANDARD) &&
       req->wIndex == 0 && req->bRequest == USB_STD_GET_DESCRIPTOR) {
        switch(req->wValue >> 8) {
        case USB_DTYPE_HID:
            dev->status.data_ptr = (uint8_t*)&(hid_cfg_desc.intf_0.hid_desc);
            dev->status.data_count = sizeof(hid_cfg_desc.intf_0.hid_desc);
            return usbd_ack;
        case USB_DTYPE_HID_REPORT:
            boot_protocol = false; /* BIOS does not read this */
            dev->status.data_ptr = (uint8_t*)hid_report_desc;
            dev->status.data_count = sizeof(hid_report_desc);
            return usbd_ack;
        default:
            return usbd_fail;
        }
    }
    return usbd_fail;
}
