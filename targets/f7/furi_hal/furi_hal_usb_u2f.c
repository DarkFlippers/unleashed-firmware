#include <furi_hal_version.h>
#include <furi_hal_usb_i.h>
#include <furi_hal_usb_hid_u2f.h>
#include <furi_hal_usb.h>
#include <furi.h>
#include "usb.h"
#include "usb_hid.h"

#define HID_PAGE_FIDO 0xF1D0
#define HID_FIDO_U2F 0x01
#define HID_FIDO_INPUT 0x20
#define HID_FIDO_OUTPUT 0x21

#define HID_EP_IN 0x81
#define HID_EP_OUT 0x01

struct HidIadDescriptor {
    struct usb_iad_descriptor hid_iad;
    struct usb_interface_descriptor hid;
    struct usb_hid_descriptor hid_desc;
    struct usb_endpoint_descriptor hid_ep_in;
    struct usb_endpoint_descriptor hid_ep_out;
};

struct HidConfigDescriptor {
    struct usb_config_descriptor config;
    struct HidIadDescriptor iad_0;
} __attribute__((packed));

/* HID report: FIDO U2F */
static const uint8_t hid_u2f_report_desc[] = {
    HID_RI_USAGE_PAGE(16, HID_PAGE_FIDO),
    HID_USAGE(HID_FIDO_U2F),
    HID_COLLECTION(HID_APPLICATION_COLLECTION),
    HID_USAGE(HID_FIDO_INPUT),
    HID_LOGICAL_MINIMUM(0x00),
    HID_RI_LOGICAL_MAXIMUM(16, 0xFF),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(HID_U2F_PACKET_LEN),
    HID_INPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_USAGE(HID_FIDO_OUTPUT),
    HID_LOGICAL_MINIMUM(0x00),
    HID_RI_LOGICAL_MAXIMUM(16, 0xFF),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(HID_U2F_PACKET_LEN),
    HID_OUTPUT(HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    HID_END_COLLECTION,
};

static const struct usb_string_descriptor dev_manuf_desc = USB_STRING_DESC("Flipper Devices Inc.");
static const struct usb_string_descriptor dev_prod_desc = USB_STRING_DESC("U2F Token");

/* Device descriptor */
static const struct usb_device_descriptor hid_u2f_device_desc = {
    .bLength = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DTYPE_DEVICE,
    .bcdUSB = VERSION_BCD(2, 0, 0),
    .bDeviceClass = USB_CLASS_IAD,
    .bDeviceSubClass = USB_SUBCLASS_IAD,
    .bDeviceProtocol = USB_PROTO_IAD,
    .bMaxPacketSize0 = USB_EP0_SIZE,
    .idVendor = 0x0483,
    .idProduct = 0x5741,
    .bcdDevice = VERSION_BCD(1, 0, 0),
    .iManufacturer = UsbDevManuf,
    .iProduct = UsbDevProduct,
    .iSerialNumber = 0,
    .bNumConfigurations = 1,
};

/* Device configuration descriptor */
static const struct HidConfigDescriptor hid_u2f_cfg_desc = {
    .config =
        {
            .bLength = sizeof(struct usb_config_descriptor),
            .bDescriptorType = USB_DTYPE_CONFIGURATION,
            .wTotalLength = sizeof(struct HidConfigDescriptor),
            .bNumInterfaces = 1,
            .bConfigurationValue = 1,
            .iConfiguration = NO_DESCRIPTOR,
            .bmAttributes = USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPOWERED,
            .bMaxPower = USB_CFG_POWER_MA(100),
        },
    .iad_0 =
        {
            .hid_iad =
                {
                    .bLength = sizeof(struct usb_iad_descriptor),
                    .bDescriptorType = USB_DTYPE_INTERFASEASSOC,
                    .bFirstInterface = 0,
                    .bInterfaceCount = 1,
                    .bFunctionClass = USB_CLASS_PER_INTERFACE,
                    .bFunctionSubClass = USB_SUBCLASS_NONE,
                    .bFunctionProtocol = USB_PROTO_NONE,
                    .iFunction = NO_DESCRIPTOR,
                },
            .hid =
                {
                    .bLength = sizeof(struct usb_interface_descriptor),
                    .bDescriptorType = USB_DTYPE_INTERFACE,
                    .bInterfaceNumber = 0,
                    .bAlternateSetting = 0,
                    .bNumEndpoints = 2,
                    .bInterfaceClass = USB_CLASS_HID,
                    .bInterfaceSubClass = USB_HID_SUBCLASS_NONBOOT,
                    .bInterfaceProtocol = USB_HID_PROTO_NONBOOT,
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
                    .wDescriptorLength0 = sizeof(hid_u2f_report_desc),
                },
            .hid_ep_in =
                {
                    .bLength = sizeof(struct usb_endpoint_descriptor),
                    .bDescriptorType = USB_DTYPE_ENDPOINT,
                    .bEndpointAddress = HID_EP_IN,
                    .bmAttributes = USB_EPTYPE_INTERRUPT,
                    .wMaxPacketSize = HID_U2F_PACKET_LEN,
                    .bInterval = 5,
                },
            .hid_ep_out =
                {
                    .bLength = sizeof(struct usb_endpoint_descriptor),
                    .bDescriptorType = USB_DTYPE_ENDPOINT,
                    .bEndpointAddress = HID_EP_OUT,
                    .bmAttributes = USB_EPTYPE_INTERRUPT,
                    .wMaxPacketSize = HID_U2F_PACKET_LEN,
                    .bInterval = 5,
                },
        },
};

static void hid_u2f_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx);
static void hid_u2f_deinit(usbd_device* dev);
static void hid_u2f_on_wakeup(usbd_device* dev);
static void hid_u2f_on_suspend(usbd_device* dev);

//static bool hid_u2f_send_report(uint8_t report_id);
static usbd_respond hid_u2f_ep_config(usbd_device* dev, uint8_t cfg);
static usbd_respond
    hid_u2f_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback);
static usbd_device* usb_dev;
static FuriSemaphore* hid_u2f_semaphore = NULL;
static bool hid_u2f_connected = false;

static HidU2fCallback callback;
static void* cb_ctx;

bool furi_hal_hid_u2f_is_connected() {
    return hid_u2f_connected;
}

void furi_hal_hid_u2f_set_callback(HidU2fCallback cb, void* ctx) {
    if(callback != NULL) {
        if(hid_u2f_connected == true) {
            callback(HidU2fDisconnected, cb_ctx);
        }
    }

    callback = cb;
    cb_ctx = ctx;

    if(callback != NULL) {
        if(hid_u2f_connected == true) {
            callback(HidU2fConnected, cb_ctx);
        }
    }
}

FuriHalUsbInterface usb_hid_u2f = {
    .init = hid_u2f_init,
    .deinit = hid_u2f_deinit,
    .wakeup = hid_u2f_on_wakeup,
    .suspend = hid_u2f_on_suspend,

    .dev_descr = (struct usb_device_descriptor*)&hid_u2f_device_desc,

    .str_manuf_descr = (void*)&dev_manuf_desc,
    .str_prod_descr = (void*)&dev_prod_desc,
    .str_serial_descr = NULL,

    .cfg_descr = (void*)&hid_u2f_cfg_desc,
};

static void hid_u2f_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx) {
    UNUSED(intf);
    UNUSED(ctx);
    if(hid_u2f_semaphore == NULL) {
        hid_u2f_semaphore = furi_semaphore_alloc(1, 1);
    }
    usb_dev = dev;

    usbd_reg_config(dev, hid_u2f_ep_config);
    usbd_reg_control(dev, hid_u2f_control);

    usbd_connect(dev, true);
}

static void hid_u2f_deinit(usbd_device* dev) {
    usbd_reg_config(dev, NULL);
    usbd_reg_control(dev, NULL);
}

static void hid_u2f_on_wakeup(usbd_device* dev) {
    UNUSED(dev);
    hid_u2f_connected = true;
    if(callback != NULL) {
        callback(HidU2fConnected, cb_ctx);
    }
}

static void hid_u2f_on_suspend(usbd_device* dev) {
    UNUSED(dev);
    if(hid_u2f_connected) {
        hid_u2f_connected = false;
        furi_semaphore_release(hid_u2f_semaphore);
        if(callback != NULL) {
            callback(HidU2fDisconnected, cb_ctx);
        }
    }
}

void furi_hal_hid_u2f_send_response(uint8_t* data, uint8_t len) {
    if((hid_u2f_semaphore == NULL) || (hid_u2f_connected == false)) return;
    furi_check(furi_semaphore_acquire(hid_u2f_semaphore, FuriWaitForever) == FuriStatusOk);
    if(hid_u2f_connected == true) {
        usbd_ep_write(usb_dev, HID_EP_OUT, data, len);
    }
}

uint32_t furi_hal_hid_u2f_get_request(uint8_t* data) {
    int32_t len = usbd_ep_read(usb_dev, HID_EP_IN, data, HID_U2F_PACKET_LEN);
    return ((len < 0) ? 0 : len);
}

static void hid_u2f_rx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
    if(callback != NULL) {
        callback(HidU2fRequest, cb_ctx);
    }
}

static void hid_u2f_tx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(event);
    UNUSED(ep);
    furi_semaphore_release(hid_u2f_semaphore);
}

static void hid_u2f_txrx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    if(event == usbd_evt_eptx) {
        hid_u2f_tx_ep_callback(dev, event, ep);
    } else {
        hid_u2f_rx_ep_callback(dev, event, ep);
    }
}

/* Configure endpoints */
static usbd_respond hid_u2f_ep_config(usbd_device* dev, uint8_t cfg) {
    switch(cfg) {
    case 0:
        /* deconfiguring device */
        usbd_ep_deconfig(dev, HID_EP_OUT);
        usbd_ep_deconfig(dev, HID_EP_IN);
        usbd_reg_endpoint(dev, HID_EP_OUT, 0);
        usbd_reg_endpoint(dev, HID_EP_IN, 0);
        return usbd_ack;
    case 1:
        /* configuring device */
        usbd_ep_config(dev, HID_EP_IN, USB_EPTYPE_INTERRUPT, HID_U2F_PACKET_LEN);
        usbd_ep_config(dev, HID_EP_OUT, USB_EPTYPE_INTERRUPT, HID_U2F_PACKET_LEN);
        usbd_reg_endpoint(dev, HID_EP_IN, hid_u2f_txrx_ep_callback);
        usbd_reg_endpoint(dev, HID_EP_OUT, hid_u2f_txrx_ep_callback);
        usbd_ep_write(dev, HID_U2F_PACKET_LEN, 0, 0);
        return usbd_ack;
    default:
        return usbd_fail;
    }
}

/* Control requests handler */
static usbd_respond
    hid_u2f_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback) {
    UNUSED(callback);
    /* HID control requests */
    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
           (USB_REQ_INTERFACE | USB_REQ_CLASS) &&
       req->wIndex == 0) {
        switch(req->bRequest) {
        case USB_HID_SETIDLE:
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
            dev->status.data_ptr = (uint8_t*)&(hid_u2f_cfg_desc.iad_0.hid_desc);
            dev->status.data_count = sizeof(hid_u2f_cfg_desc.iad_0.hid_desc);
            return usbd_ack;
        case USB_DTYPE_HID_REPORT:
            dev->status.data_ptr = (uint8_t*)hid_u2f_report_desc;
            dev->status.data_count = sizeof(hid_u2f_report_desc);
            return usbd_ack;
        default:
            return usbd_fail;
        }
    }
    return usbd_fail;
}
