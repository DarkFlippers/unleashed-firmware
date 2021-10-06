#include "furi-hal-version.h"
#include "furi-hal-usb_i.h"
#include <furi.h>

#include "usb.h"
#include "usb_hid.h"
#include "hid_usage_desktop.h"
#include "hid_usage_button.h"

#define HID_RIN_EP      0x81
#define HID_RIN_SZ      0x10

struct HidIadDescriptor {
    struct usb_iad_descriptor           hid_iad;
    struct usb_interface_descriptor     hid;
    struct usb_hid_descriptor           hid_desc;
    struct usb_endpoint_descriptor      hid_ep;    
};

struct HidConfigDescriptor {
    struct usb_config_descriptor        config;
    struct HidIadDescriptor             iad_0;
} __attribute__((packed));

/* HID mouse report desscriptor. 2 axis 5 buttons */
static const uint8_t hid_report_desc[] = {
    HID_USAGE_PAGE(HID_PAGE_DESKTOP),
    HID_USAGE(HID_DESKTOP_MOUSE),
    HID_COLLECTION(HID_APPLICATION_COLLECTION),
        HID_USAGE(HID_DESKTOP_POINTER),
        HID_COLLECTION(HID_PHYSICAL_COLLECTION),
            HID_USAGE(HID_DESKTOP_X),
            HID_USAGE(HID_DESKTOP_Y),
            HID_LOGICAL_MINIMUM(-127),
            HID_LOGICAL_MAXIMUM(127),
            HID_REPORT_SIZE(8),
            HID_REPORT_COUNT(2),
            HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_RELATIVE ),
            HID_USAGE_PAGE(HID_PAGE_BUTTON),
            HID_USAGE_MINIMUM(1),
            HID_USAGE_MAXIMUM(5),
            HID_LOGICAL_MINIMUM(0),
            HID_LOGICAL_MAXIMUM(1),
            HID_REPORT_SIZE(1),
            HID_REPORT_COUNT(5),
            HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE ),
            HID_REPORT_SIZE(1),
            HID_REPORT_COUNT(3),
            HID_INPUT(HID_IOF_CONSTANT),
        HID_END_COLLECTION,
    HID_END_COLLECTION,
};

static const struct usb_string_descriptor dev_manuf_desc = USB_STRING_DESC("Logitech");
static const struct usb_string_descriptor dev_prod_desc = USB_STRING_DESC("USB Receiver");
static const struct usb_string_descriptor dev_serial_desc = USB_STRING_DESC("1234567890");

/* Device descriptor */
static const struct usb_device_descriptor hid_device_desc = {
    .bLength            = sizeof(struct usb_device_descriptor),
    .bDescriptorType    = USB_DTYPE_DEVICE,
    .bcdUSB             = VERSION_BCD(2,0,0),
    .bDeviceClass       = USB_CLASS_IAD,
    .bDeviceSubClass    = USB_SUBCLASS_IAD,
    .bDeviceProtocol    = USB_PROTO_IAD,
    .bMaxPacketSize0    = USB_EP0_SIZE,
    .idVendor           = 0x046d,
    .idProduct          = 0xc529,
    .bcdDevice          = VERSION_BCD(1,0,0),
    .iManufacturer      = UsbDevManuf,
    .iProduct           = UsbDevProduct,
    .iSerialNumber      = UsbDevSerial,
    .bNumConfigurations = 1,
};

/* Device configuration descriptor */
static const struct HidConfigDescriptor hid_cfg_desc = {
    .config = {
        .bLength                = sizeof(struct usb_config_descriptor),
        .bDescriptorType        = USB_DTYPE_CONFIGURATION,
        .wTotalLength           = sizeof(struct HidConfigDescriptor),
        .bNumInterfaces         = 1,
        .bConfigurationValue    = 1,
        .iConfiguration         = NO_DESCRIPTOR,
        .bmAttributes           = USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPOWERED,
        .bMaxPower              = USB_CFG_POWER_MA(100),
    },
    .iad_0 = {
        .hid_iad = {
            .bLength = sizeof(struct usb_iad_descriptor),
            .bDescriptorType        = USB_DTYPE_INTERFASEASSOC,
            .bFirstInterface        = 0,
            .bInterfaceCount        = 1,
            .bFunctionClass         = USB_CLASS_PER_INTERFACE,
            .bFunctionSubClass      = USB_SUBCLASS_NONE,
            .bFunctionProtocol      = USB_PROTO_NONE,
            .iFunction              = NO_DESCRIPTOR,
        },
        .hid = {
            .bLength                = sizeof(struct usb_interface_descriptor),
            .bDescriptorType        = USB_DTYPE_INTERFACE,
            .bInterfaceNumber       = 0,
            .bAlternateSetting      = 0,
            .bNumEndpoints          = 1,
            .bInterfaceClass        = USB_CLASS_HID,
            .bInterfaceSubClass     = USB_HID_SUBCLASS_NONBOOT,
            .bInterfaceProtocol     = USB_HID_PROTO_NONBOOT,
            .iInterface             = NO_DESCRIPTOR,
        },
        .hid_desc = {
            .bLength                = sizeof(struct usb_hid_descriptor),
            .bDescriptorType        = USB_DTYPE_HID,
            .bcdHID                 = VERSION_BCD(1,0,0),
            .bCountryCode           = USB_HID_COUNTRY_NONE,
            .bNumDescriptors        = 1,
            .bDescriptorType0       = USB_DTYPE_HID_REPORT,
            .wDescriptorLength0     = sizeof(hid_report_desc),
        },
        .hid_ep = {
            .bLength                = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType        = USB_DTYPE_ENDPOINT,
            .bEndpointAddress       = HID_RIN_EP,
            .bmAttributes           = USB_EPTYPE_INTERRUPT,
            .wMaxPacketSize         = HID_RIN_SZ,
            .bInterval              = 50,
        },
    },
};

static struct {
    int8_t      x;
    int8_t      y;
    uint8_t     buttons;
} __attribute__((packed)) hid_report_data;

static void hid_init(usbd_device* dev, struct UsbInterface* intf);
static void hid_deinit(usbd_device *dev);
static void hid_on_wakeup(usbd_device *dev);
static void hid_on_suspend(usbd_device *dev);

static usbd_respond hid_ep_config (usbd_device *dev, uint8_t cfg);
static usbd_respond hid_control (usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback);
static usbd_device* usb_dev;

struct UsbInterface usb_hid = {
    .init = hid_init,
    .deinit = hid_deinit,
    .wakeup = hid_on_wakeup,
    .suspend = hid_on_suspend,

    .dev_descr = (struct usb_device_descriptor*)&hid_device_desc,    

    .str_manuf_descr = (void*)&dev_manuf_desc,
    .str_prod_descr = (void*)&dev_prod_desc,
    .str_serial_descr = (void*)&dev_serial_desc,

    .cfg_descr = (void*)&hid_cfg_desc,
};

static void hid_init(usbd_device* dev, struct UsbInterface* intf) {
    usb_dev = dev;

    usbd_reg_config(dev, hid_ep_config);
    usbd_reg_control(dev, hid_control);    

    usbd_connect(dev, true);
}

static void hid_deinit(usbd_device *dev) {
    usbd_reg_config(dev, NULL);
    usbd_reg_control(dev, NULL);
}

static void hid_on_wakeup(usbd_device *dev) {
}

static void hid_on_suspend(usbd_device *dev) {
}

/* HID mouse IN endpoint callback */
static void hid_mouse_move(usbd_device *dev, uint8_t event, uint8_t ep) {
    static uint8_t t = 0;
    if (t < 0x10) {
        hid_report_data.x = 1;
        hid_report_data.y = 0;
    } else if (t < 0x20) {
        hid_report_data.x = 1;
        hid_report_data.y = 1;
    } else if (t < 0x30) {
        hid_report_data.x = 0;
        hid_report_data.y = 1;
    } else if (t < 0x40) {
        hid_report_data.x = -1;
        hid_report_data.y = 1;
    } else if (t < 0x50) {
        hid_report_data.x = -1;
        hid_report_data.y = 0;
    } else if (t < 0x60) {
        hid_report_data.x = -1;
        hid_report_data.y = -1;
    } else if (t < 0x70) {
        hid_report_data.x = 0;
        hid_report_data.y = -1;
    } else  {
        hid_report_data.x = 1;
        hid_report_data.y = -1;
    }
    t = (t + 1) & 0x7F;
    usbd_ep_write(dev, ep, &hid_report_data, sizeof(hid_report_data));
}

/* Configure endpoints */
static usbd_respond hid_ep_config (usbd_device *dev, uint8_t cfg) {
    switch (cfg) {
    case 0:
        /* deconfiguring device */
        usbd_ep_deconfig(dev, HID_RIN_EP);
        usbd_reg_endpoint(dev, HID_RIN_EP, 0);
        return usbd_ack;
    case 1:
        /* configuring device */
        usbd_ep_config(dev, HID_RIN_EP, USB_EPTYPE_INTERRUPT, HID_RIN_SZ);
        usbd_reg_endpoint(dev, HID_RIN_EP, hid_mouse_move);
        usbd_ep_write(dev, HID_RIN_EP, 0, 0);
        return usbd_ack;
    default:
        return usbd_fail;
    }
}

/* Control requests handler */
static usbd_respond hid_control (usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback) {
    /* HID control requests */
    if (((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) == (USB_REQ_INTERFACE | USB_REQ_CLASS)
        && req->wIndex == 0 ) {
        switch (req->bRequest) {
        case USB_HID_SETIDLE:
            return usbd_ack;
        case USB_HID_GETREPORT:
            dev->status.data_ptr = &hid_report_data;
            dev->status.data_count = sizeof(hid_report_data);
            return usbd_ack;
        default:
            return usbd_fail;
        }
    }
    if (((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) == (USB_REQ_INTERFACE | USB_REQ_STANDARD)
        && req->wIndex == 0
        && req->bRequest == USB_STD_GET_DESCRIPTOR) {
        switch (req->wValue >> 8) {
        case USB_DTYPE_HID:
            dev->status.data_ptr = (uint8_t*)&(hid_cfg_desc.iad_0.hid_desc);
            dev->status.data_count = sizeof(hid_cfg_desc.iad_0.hid_desc);
            return usbd_ack;
        case USB_DTYPE_HID_REPORT:
            dev->status.data_ptr = (uint8_t*)hid_report_desc;
            dev->status.data_count = sizeof(hid_report_desc);
            return usbd_ack;
        default:
            return usbd_fail;
        }
    }
    return usbd_fail;
}
