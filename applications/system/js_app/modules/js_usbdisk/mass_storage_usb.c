#include "mass_storage_usb.h"
#include <furi_hal.h>

#define TAG "MassStorageUsb"

#define USB_MSC_RX_EP (0x01)
#define USB_MSC_TX_EP (0x82)

#define USB_MSC_RX_EP_SIZE (64UL)
#define USB_MSC_TX_EP_SIZE (64UL)

#define USB_MSC_BOT_GET_MAX_LUN (0xFE)
#define USB_MSC_BOT_RESET       (0xFF)

#define CBW_SIG                  (0x43425355)
#define CBW_FLAGS_DEVICE_TO_HOST (0x80)

#define CSW_SIG                (0x53425355)
#define CSW_STATUS_OK          (0)
#define CSW_STATUS_NOK         (1)
#define CSW_STATUS_PHASE_ERROR (2)

// must be SCSI_BLOCK_SIZE aligned
// larger than 0x10000 exceeds size_t, storage_file_* ops fail
#define USB_MSC_BUF_MAX (0x10000UL - SCSI_BLOCK_SIZE)

static usbd_respond usb_ep_config(usbd_device* dev, uint8_t cfg);
static usbd_respond usb_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback);

typedef enum {
    EventExit = 1 << 0,
    EventReset = 1 << 1,
    EventRxTx = 1 << 2,

    EventAll = EventExit | EventReset | EventRxTx,
} MassStorageEvent;

typedef struct {
    uint32_t sig;
    uint32_t tag;
    uint32_t len;
    uint8_t flags;
    uint8_t lun;
    uint8_t cmd_len;
    uint8_t cmd[16];
} __attribute__((packed)) CBW;

typedef struct {
    uint32_t sig;
    uint32_t tag;
    uint32_t residue;
    uint8_t status;
} __attribute__((packed)) CSW;

struct MassStorageUsb {
    FuriHalUsbInterface usb;
    FuriHalUsbInterface* usb_prev;

    FuriThread* thread;
    usbd_device* dev;
    SCSIDeviceFunc fn;
};

static int32_t mass_thread_worker(void* context) {
    MassStorageUsb* mass = context;
    usbd_device* dev = mass->dev;
    SCSISession scsi = {
        .fn = mass->fn,
    };
    CBW cbw = {0};
    CSW csw = {0};
    uint8_t* buf = NULL;
    uint32_t buf_len = 0, buf_cap = 0, buf_sent = 0;
    enum {
        StateReadCBW,
        StateReadData,
        StateWriteData,
        StateBuildCSW,
        StateWriteCSW,
    } state = StateReadCBW;
    while(true) {
        uint32_t flags = furi_thread_flags_wait(EventAll, FuriFlagWaitAny, FuriWaitForever);
        if(flags & EventExit) {
            FURI_LOG_D(TAG, "exit");
            break;
        }
        if(flags & EventReset) {
            FURI_LOG_D(TAG, "reset");
            scsi.sk = 0;
            scsi.asc = 0;
            memset(&cbw, 0, sizeof(cbw));
            memset(&csw, 0, sizeof(csw));
            if(buf) {
                free(buf);
                buf = NULL;
            }
            buf_len = buf_cap = buf_sent = 0;
            state = StateReadCBW;
            mass->fn.eject(mass->fn.ctx);
        }
        if(flags & EventRxTx) do {
                switch(state) {
                case StateReadCBW: {
                    FURI_LOG_T(TAG, "StateReadCBW");
                    int32_t len = usbd_ep_read(dev, USB_MSC_RX_EP, &cbw, sizeof(cbw));
                    if(len <= 0) {
                        FURI_LOG_T(TAG, "cbw not ready");
                        break;
                    }
                    if(len != sizeof(cbw) || cbw.sig != CBW_SIG) {
                        FURI_LOG_W(TAG, "bad cbw sig=%08lx", cbw.sig);
                        usbd_ep_stall(dev, USB_MSC_TX_EP);
                        usbd_ep_stall(dev, USB_MSC_RX_EP);
                        continue;
                    }
                    if(!scsi_cmd_start(&scsi, cbw.cmd, cbw.cmd_len)) {
                        FURI_LOG_W(TAG, "bad cmd");
                        usbd_ep_stall(dev, USB_MSC_RX_EP);
                        csw.sig = CSW_SIG;
                        csw.tag = cbw.tag;
                        csw.status = CSW_STATUS_NOK;
                        state = StateWriteCSW;
                        continue;
                    }
                    if(cbw.flags & CBW_FLAGS_DEVICE_TO_HOST) {
                        buf_len = 0;
                        buf_sent = 0;
                        state = StateWriteData;
                    } else {
                        buf_len = 0;
                        state = StateReadData;
                    }
                    continue;
                }; break;
                case StateReadData: {
                    FURI_LOG_T(TAG, "StateReadData %lu/%lu", buf_len, cbw.len);
                    if(!cbw.len) {
                        state = StateBuildCSW;
                        continue;
                    }
                    uint32_t buf_clamp = MIN(cbw.len, USB_MSC_BUF_MAX);
                    if(buf_clamp > buf_cap) {
                        FURI_LOG_T(TAG, "growing buf %lu -> %lu", buf_cap, buf_clamp);
                        if(buf) {
                            free(buf);
                        }
                        buf_cap = buf_clamp;
                        buf = malloc(buf_cap);
                    }
                    if(buf_len < buf_clamp) {
                        int32_t len =
                            usbd_ep_read(dev, USB_MSC_RX_EP, buf + buf_len, buf_clamp - buf_len);
                        if(len < 0) {
                            FURI_LOG_T(TAG, "rx not ready %ld", len);
                            break;
                        }
                        FURI_LOG_T(TAG, "clamp %lu len %ld", buf_clamp, len);
                        buf_len += len;
                    }
                    if(buf_len == buf_clamp) {
                        if(!scsi_cmd_rx_data(&scsi, buf, buf_len)) {
                            FURI_LOG_W(TAG, "short rx");
                            usbd_ep_stall(dev, USB_MSC_RX_EP);
                            csw.sig = CSW_SIG;
                            csw.tag = cbw.tag;
                            csw.status = CSW_STATUS_NOK;
                            csw.residue = cbw.len;
                            state = StateWriteCSW;
                            continue;
                        }
                        cbw.len -= buf_len;
                        buf_len = 0;
                    }
                    continue;
                }; break;
                case StateWriteData: {
                    FURI_LOG_T(TAG, "StateWriteData %lu", cbw.len);
                    if(!cbw.len) {
                        state = StateBuildCSW;
                        continue;
                    }
                    uint32_t buf_clamp = MIN(cbw.len, USB_MSC_BUF_MAX);
                    if(buf_clamp > buf_cap) {
                        FURI_LOG_T(TAG, "growing buf %lu -> %lu", buf_cap, buf_clamp);
                        if(buf) {
                            free(buf);
                        }
                        buf_cap = buf_clamp;
                        buf = malloc(buf_cap);
                    }
                    if(!buf_len && !scsi_cmd_tx_data(&scsi, buf, &buf_len, buf_clamp)) {
                        FURI_LOG_W(TAG, "short tx");
                        // usbd_ep_stall(dev, USB_MSC_TX_EP);
                        state = StateBuildCSW;
                        continue;
                    }
                    int32_t len = usbd_ep_write(
                        dev,
                        USB_MSC_TX_EP,
                        buf + buf_sent,
                        MIN(USB_MSC_TX_EP_SIZE, buf_len - buf_sent));
                    if(len < 0) {
                        FURI_LOG_T(TAG, "tx not ready %ld", len);
                        break;
                    }
                    buf_sent += len;
                    if(buf_sent == buf_len) {
                        cbw.len -= buf_len;
                        buf_len = 0;
                        buf_sent = 0;
                    }
                    continue;
                }; break;
                case StateBuildCSW: {
                    FURI_LOG_T(TAG, "StateBuildCSW");
                    csw.sig = CSW_SIG;
                    csw.tag = cbw.tag;
                    if(scsi_cmd_end(&scsi)) {
                        csw.status = CSW_STATUS_OK;
                    } else {
                        csw.status = CSW_STATUS_NOK;
                    }
                    csw.residue = cbw.len;
                    state = StateWriteCSW;
                    continue;
                }; break;
                case StateWriteCSW: {
                    FURI_LOG_T(TAG, "StateWriteCSW");
                    if(csw.status) {
                        FURI_LOG_W(
                            TAG,
                            "csw sig=%08lx tag=%08lx residue=%08lx status=%02x",
                            csw.sig,
                            csw.tag,
                            csw.residue,
                            csw.status);
                    }
                    int32_t len = usbd_ep_write(dev, USB_MSC_TX_EP, &csw, sizeof(csw));
                    if(len < 0) {
                        FURI_LOG_T(TAG, "csw not ready");
                        break;
                    }
                    if(len != sizeof(csw)) {
                        FURI_LOG_W(TAG, "bad csw write %ld", len);
                        usbd_ep_stall(dev, USB_MSC_TX_EP);
                        break;
                    }
                    memset(&cbw, 0, sizeof(cbw));
                    memset(&csw, 0, sizeof(csw));
                    state = StateReadCBW;
                    continue;
                }; break;
                }
                break;
            } while(true);
    }
    if(buf) {
        free(buf);
    }
    return 0;
}

// needed in usb_deinit, usb_suspend, usb_rxtx_ep_callback, usb_control,
// where if_ctx isn't passed
static MassStorageUsb* mass_cur = NULL;

static void usb_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx) {
    UNUSED(intf);
    MassStorageUsb* mass = ctx;
    mass_cur = mass;
    mass->dev = dev;

    usbd_reg_config(dev, usb_ep_config);
    usbd_reg_control(dev, usb_control);
    usbd_connect(dev, true);

    mass->thread = furi_thread_alloc();
    furi_thread_set_name(mass->thread, "MassStorageUsb");
    furi_thread_set_stack_size(mass->thread, 1024);
    furi_thread_set_context(mass->thread, ctx);
    furi_thread_set_callback(mass->thread, mass_thread_worker);
    furi_thread_start(mass->thread);
}

static void usb_deinit(usbd_device* dev) {
    usbd_reg_config(dev, NULL);
    usbd_reg_control(dev, NULL);

    MassStorageUsb* mass = mass_cur;
    if(!mass || mass->dev != dev) {
        FURI_LOG_E(TAG, "deinit mass_cur leak");
        return;
    }
    mass_cur = NULL;

    furi_assert(mass->thread);
    furi_thread_flags_set(furi_thread_get_id(mass->thread), EventExit);
    furi_thread_join(mass->thread);
    furi_thread_free(mass->thread);
    mass->thread = NULL;

    free(mass->usb.str_prod_descr);
    mass->usb.str_prod_descr = NULL;
    free(mass->usb.str_serial_descr);
    mass->usb.str_serial_descr = NULL;
    free(mass);
}

static void usb_wakeup(usbd_device* dev) {
    UNUSED(dev);
}

static void usb_suspend(usbd_device* dev) {
    MassStorageUsb* mass = mass_cur;
    if(!mass || mass->dev != dev) return;
    furi_thread_flags_set(furi_thread_get_id(mass->thread), EventReset);
}

static void usb_rxtx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(ep);
    UNUSED(event);
    MassStorageUsb* mass = mass_cur;
    if(!mass || mass->dev != dev) return;
    furi_thread_flags_set(furi_thread_get_id(mass->thread), EventRxTx);
}

static usbd_respond usb_ep_config(usbd_device* dev, uint8_t cfg) {
    switch(cfg) {
    case 0: // deconfig
        usbd_ep_deconfig(dev, USB_MSC_RX_EP);
        usbd_ep_deconfig(dev, USB_MSC_TX_EP);
        usbd_reg_endpoint(dev, USB_MSC_RX_EP, NULL);
        usbd_reg_endpoint(dev, USB_MSC_TX_EP, NULL);
        return usbd_ack;
    case 1: // config
        usbd_ep_config(
            dev, USB_MSC_RX_EP, USB_EPTYPE_BULK /* | USB_EPTYPE_DBLBUF*/, USB_MSC_RX_EP_SIZE);
        usbd_ep_config(
            dev, USB_MSC_TX_EP, USB_EPTYPE_BULK /* | USB_EPTYPE_DBLBUF*/, USB_MSC_TX_EP_SIZE);
        usbd_reg_endpoint(dev, USB_MSC_RX_EP, usb_rxtx_ep_callback);
        usbd_reg_endpoint(dev, USB_MSC_TX_EP, usb_rxtx_ep_callback);
        return usbd_ack;
    }
    return usbd_fail;
}

static usbd_respond usb_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback) {
    UNUSED(callback);
    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) !=
       (USB_REQ_INTERFACE | USB_REQ_CLASS)) {
        return usbd_fail;
    }
    switch(req->bRequest) {
    case USB_MSC_BOT_GET_MAX_LUN: {
        static uint8_t max_lun = 0;
        dev->status.data_ptr = &max_lun;
        dev->status.data_count = 1;
        return usbd_ack;
    }; break;
    case USB_MSC_BOT_RESET: {
        MassStorageUsb* mass = mass_cur;
        if(!mass || mass->dev != dev) return usbd_fail;
        furi_thread_flags_set(furi_thread_get_id(mass->thread), EventReset);
        return usbd_ack;
    }; break;
    }
    return usbd_fail;
}

static const struct usb_string_descriptor dev_manuf_desc = USB_STRING_DESC("Flipper Devices Inc.");

struct MassStorageDescriptor {
    struct usb_config_descriptor config;
    struct usb_interface_descriptor intf;
    struct usb_endpoint_descriptor ep_rx;
    struct usb_endpoint_descriptor ep_tx;
} __attribute__((packed));

static const struct usb_device_descriptor usb_mass_dev_descr = {
    .bLength = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DTYPE_DEVICE,
    .bcdUSB = VERSION_BCD(2, 0, 0),
    .bDeviceClass = USB_CLASS_PER_INTERFACE,
    .bDeviceSubClass = USB_SUBCLASS_NONE,
    .bDeviceProtocol = USB_PROTO_NONE,
    .bMaxPacketSize0 = 8, // USB_EP0_SIZE
    .idVendor = 0x0483,
    .idProduct = 0x5720,
    .bcdDevice = VERSION_BCD(1, 0, 0),
    .iManufacturer = 1, // UsbDevManuf
    .iProduct = 2, // UsbDevProduct
    .iSerialNumber = 3, // UsbDevSerial
    .bNumConfigurations = 1,
};

static const struct MassStorageDescriptor usb_mass_cfg_descr = {
    .config =
        {
            .bLength = sizeof(struct usb_config_descriptor),
            .bDescriptorType = USB_DTYPE_CONFIGURATION,
            .wTotalLength = sizeof(struct MassStorageDescriptor),
            .bNumInterfaces = 1,
            .bConfigurationValue = 1,
            .iConfiguration = NO_DESCRIPTOR,
            .bmAttributes = USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPOWERED,
            .bMaxPower = USB_CFG_POWER_MA(100),
        },
    .intf =
        {
            .bLength = sizeof(struct usb_interface_descriptor),
            .bDescriptorType = USB_DTYPE_INTERFACE,
            .bInterfaceNumber = 0,
            .bAlternateSetting = 0,
            .bNumEndpoints = 2,
            .bInterfaceClass = USB_CLASS_MASS_STORAGE,
            .bInterfaceSubClass = 0x06, // scsi transparent
            .bInterfaceProtocol = 0x50, // bulk only
            .iInterface = NO_DESCRIPTOR,
        },
    .ep_rx =
        {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = USB_MSC_RX_EP,
            .bmAttributes = USB_EPTYPE_BULK,
            .wMaxPacketSize = USB_MSC_RX_EP_SIZE,
            .bInterval = 0,
        },
    .ep_tx =
        {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = USB_MSC_TX_EP,
            .bmAttributes = USB_EPTYPE_BULK,
            .wMaxPacketSize = USB_MSC_TX_EP_SIZE,
            .bInterval = 0,
        },
};

MassStorageUsb* mass_storage_usb_start(const char* filename, SCSIDeviceFunc fn) {
    MassStorageUsb* mass = malloc(sizeof(MassStorageUsb));
    mass->usb_prev = furi_hal_usb_get_config();
    mass->usb.init = usb_init;
    mass->usb.deinit = usb_deinit;
    mass->usb.wakeup = usb_wakeup;
    mass->usb.suspend = usb_suspend;
    mass->usb.dev_descr = (struct usb_device_descriptor*)&usb_mass_dev_descr;
    mass->usb.str_manuf_descr = (void*)&dev_manuf_desc;
    mass->usb.str_prod_descr = NULL;
    mass->usb.str_serial_descr = NULL;
    mass->usb.cfg_descr = (void*)&usb_mass_cfg_descr;

    const char* name = furi_hal_version_get_device_name_ptr();
    if(!name) name = "Flipper Zero";
    size_t len = strlen(name);
    struct usb_string_descriptor* str_prod_descr = malloc(len * 2 + 2);
    str_prod_descr->bLength = len * 2 + 2;
    str_prod_descr->bDescriptorType = USB_DTYPE_STRING;
    for(uint8_t i = 0; i < len; i++)
        str_prod_descr->wString[i] = name[i];
    mass->usb.str_prod_descr = str_prod_descr;

    len = strlen(filename);
    struct usb_string_descriptor* str_serial_descr = malloc(len * 2 + 2);
    str_serial_descr->bLength = len * 2 + 2;
    str_serial_descr->bDescriptorType = USB_DTYPE_STRING;
    for(uint8_t i = 0; i < len; i++)
        str_serial_descr->wString[i] = filename[i];
    mass->usb.str_serial_descr = str_serial_descr;

    mass->fn = fn;
    if(!furi_hal_usb_set_config(&mass->usb, mass)) {
        FURI_LOG_E(TAG, "USB locked, cannot start Mass Storage");
        free(mass->usb.str_prod_descr);
        free(mass->usb.str_serial_descr);
        free(mass);
        return NULL;
    }
    return mass;
}

void mass_storage_usb_stop(MassStorageUsb* mass) {
    furi_hal_usb_set_config(mass->usb_prev, NULL);
}
