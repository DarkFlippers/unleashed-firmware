#include <furi.h>
#include <usb.h>
#include <usb_std.h>
#include <usb_hid.h>
#include <usb_cdc.h>
#include <furi_hal_console.h>

#include "dap_v2_usb.h"

// #define DAP_USB_LOG

#define HID_EP_IN 0x80
#define HID_EP_OUT 0x00

#define DAP_HID_EP_SEND 1
#define DAP_HID_EP_RECV 2
#define DAP_HID_EP_BULK_RECV 3
#define DAP_HID_EP_BULK_SEND 4
#define DAP_CDC_EP_COMM 5
#define DAP_CDC_EP_SEND 6
#define DAP_CDC_EP_RECV 7

#define DAP_HID_EP_IN (HID_EP_IN | DAP_HID_EP_SEND)
#define DAP_HID_EP_OUT (HID_EP_OUT | DAP_HID_EP_RECV)
#define DAP_HID_EP_BULK_IN (HID_EP_IN | DAP_HID_EP_BULK_SEND)
#define DAP_HID_EP_BULK_OUT (HID_EP_OUT | DAP_HID_EP_BULK_RECV)

#define DAP_HID_EP_SIZE 64
#define DAP_CDC_COMM_EP_SIZE 8
#define DAP_CDC_EP_SIZE 64

#define DAP_BULK_INTERVAL 0
#define DAP_HID_INTERVAL 1
#define DAP_CDC_INTERVAL 0
#define DAP_CDC_COMM_INTERVAL 1

#define DAP_HID_VID 0x0483
#define DAP_HID_PID 0x5740

#define DAP_USB_EP0_SIZE 8

#define EP_CFG_DECONFIGURE 0
#define EP_CFG_CONFIGURE 1

enum {
    USB_INTF_HID,
    USB_INTF_BULK,
    USB_INTF_CDC_COMM,
    USB_INTF_CDC_DATA,
    USB_INTF_COUNT,
};

enum {
    USB_STR_ZERO,
    USB_STR_MANUFACTURER,
    USB_STR_PRODUCT,
    USB_STR_SERIAL_NUMBER,
    USB_STR_CMSIS_DAP_V1,
    USB_STR_CMSIS_DAP_V2,
    USB_STR_COM_PORT,
    USB_STR_COUNT,
};

// static const char* usb_str[] = {
//     [USB_STR_MANUFACTURER] = "Flipper Devices Inc.",
//     [USB_STR_PRODUCT] = "Combined VCP and CMSIS-DAP Adapter",
//     [USB_STR_COM_PORT] = "Virtual COM-Port",
//     [USB_STR_CMSIS_DAP_V1] = "CMSIS-DAP v1 Adapter",
//     [USB_STR_CMSIS_DAP_V2] = "CMSIS-DAP v2 Adapter",
//     [USB_STR_SERIAL_NUMBER] = "01234567890ABCDEF",
// };

static const struct usb_string_descriptor dev_manuf_descr =
    USB_STRING_DESC("Flipper Devices Inc.");

static const struct usb_string_descriptor dev_prod_descr =
    USB_STRING_DESC("Combined VCP and CMSIS-DAP Adapter");

static struct usb_string_descriptor* dev_serial_descr = NULL;

static const struct usb_string_descriptor dev_dap_v1_descr =
    USB_STRING_DESC("CMSIS-DAP v1 Adapter");

static const struct usb_string_descriptor dev_dap_v2_descr =
    USB_STRING_DESC("CMSIS-DAP v2 Adapter");

static const struct usb_string_descriptor dev_com_descr = USB_STRING_DESC("Virtual COM-Port");

struct HidConfigDescriptor {
    struct usb_config_descriptor configuration;

    // CMSIS-DAP v1
    struct usb_interface_descriptor hid_interface;
    struct usb_hid_descriptor hid;
    struct usb_endpoint_descriptor hid_ep_in;
    struct usb_endpoint_descriptor hid_ep_out;

    // CMSIS-DAP v2
    struct usb_interface_descriptor bulk_interface;
    struct usb_endpoint_descriptor bulk_ep_out;
    struct usb_endpoint_descriptor bulk_ep_in;

    // CDC
    struct usb_iad_descriptor iad;
    struct usb_interface_descriptor interface_comm;
    struct usb_cdc_header_desc cdc_header;
    struct usb_cdc_call_mgmt_desc cdc_acm;
    struct usb_cdc_acm_desc cdc_call_mgmt;
    struct usb_cdc_union_desc cdc_union;
    struct usb_endpoint_descriptor ep_comm;
    struct usb_interface_descriptor interface_data;
    struct usb_endpoint_descriptor ep_in;
    struct usb_endpoint_descriptor ep_out;

} __attribute__((packed));

static const struct usb_device_descriptor hid_device_desc = {
    .bLength = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DTYPE_DEVICE,
    .bcdUSB = VERSION_BCD(2, 1, 0),
    .bDeviceClass = USB_CLASS_MISC,
    .bDeviceSubClass = USB_SUBCLASS_IAD,
    .bDeviceProtocol = USB_PROTO_IAD,
    .bMaxPacketSize0 = DAP_USB_EP0_SIZE,
    .idVendor = DAP_HID_VID,
    .idProduct = DAP_HID_PID,
    .bcdDevice = VERSION_BCD(1, 0, 0),
    .iManufacturer = USB_STR_MANUFACTURER,
    .iProduct = USB_STR_PRODUCT,
    .iSerialNumber = USB_STR_SERIAL_NUMBER,
    .bNumConfigurations = 1,
};

static const uint8_t hid_report_desc[] = {
    0x05, 0x01, // Usage Page (Generic Desktop Ctrls)
    0x09, 0x00, // Usage (Undefined)
    0xa1, 0x01, // Collection (Application)
    0x15, 0x00, //   Logical Minimum (0)
    0x26, 0xff, 0x00, //   Logical Maximum (255)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x40, //   Report Count (64)
    0x09, 0x00, //   Usage (Undefined)
    0x81, 0x82, //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08, //   Report Size (8)
    0x95, 0x40, //   Report Count (64)
    0x09, 0x00, //   Usage (Undefined)
    0x91, 0x82, //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)
    0xc0, // End Collection
};

static const struct HidConfigDescriptor hid_cfg_desc = {
    .configuration =
        {
            .bLength = sizeof(struct usb_config_descriptor),
            .bDescriptorType = USB_DTYPE_CONFIGURATION,
            .wTotalLength = sizeof(struct HidConfigDescriptor),
            .bNumInterfaces = USB_INTF_COUNT,
            .bConfigurationValue = 1,
            .iConfiguration = NO_DESCRIPTOR,
            .bmAttributes = USB_CFG_ATTR_RESERVED,
            .bMaxPower = USB_CFG_POWER_MA(500),
        },

    // CMSIS-DAP v1
    .hid_interface =
        {
            .bLength = sizeof(struct usb_interface_descriptor),
            .bDescriptorType = USB_DTYPE_INTERFACE,
            .bInterfaceNumber = USB_INTF_HID,
            .bAlternateSetting = 0,
            .bNumEndpoints = 2,
            .bInterfaceClass = USB_CLASS_HID,
            .bInterfaceSubClass = USB_HID_SUBCLASS_NONBOOT,
            .bInterfaceProtocol = USB_HID_PROTO_NONBOOT,
            .iInterface = USB_STR_CMSIS_DAP_V1,
        },

    .hid =
        {
            .bLength = sizeof(struct usb_hid_descriptor),
            .bDescriptorType = USB_DTYPE_HID,
            .bcdHID = VERSION_BCD(1, 1, 1),
            .bCountryCode = USB_HID_COUNTRY_NONE,
            .bNumDescriptors = 1,
            .bDescriptorType0 = USB_DTYPE_HID_REPORT,
            .wDescriptorLength0 = sizeof(hid_report_desc),
        },

    .hid_ep_in =
        {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = DAP_HID_EP_IN,
            .bmAttributes = USB_EPTYPE_INTERRUPT,
            .wMaxPacketSize = DAP_HID_EP_SIZE,
            .bInterval = DAP_HID_INTERVAL,
        },

    .hid_ep_out =
        {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = DAP_HID_EP_OUT,
            .bmAttributes = USB_EPTYPE_INTERRUPT,
            .wMaxPacketSize = DAP_HID_EP_SIZE,
            .bInterval = DAP_HID_INTERVAL,
        },

    // CMSIS-DAP v2
    .bulk_interface =
        {
            .bLength = sizeof(struct usb_interface_descriptor),
            .bDescriptorType = USB_DTYPE_INTERFACE,
            .bInterfaceNumber = USB_INTF_BULK,
            .bAlternateSetting = 0,
            .bNumEndpoints = 2,
            .bInterfaceClass = USB_CLASS_VENDOR,
            .bInterfaceSubClass = 0,
            .bInterfaceProtocol = 0,
            .iInterface = USB_STR_CMSIS_DAP_V2,
        },

    .bulk_ep_out =
        {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = DAP_HID_EP_BULK_OUT,
            .bmAttributes = USB_EPTYPE_BULK,
            .wMaxPacketSize = DAP_HID_EP_SIZE,
            .bInterval = DAP_BULK_INTERVAL,
        },

    .bulk_ep_in =
        {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = DAP_HID_EP_BULK_IN,
            .bmAttributes = USB_EPTYPE_BULK,
            .wMaxPacketSize = DAP_HID_EP_SIZE,
            .bInterval = DAP_BULK_INTERVAL,
        },

    // CDC
    .iad =
        {
            .bLength = sizeof(struct usb_iad_descriptor),
            .bDescriptorType = USB_DTYPE_INTERFASEASSOC,
            .bFirstInterface = USB_INTF_CDC_COMM,
            .bInterfaceCount = 2,
            .bFunctionClass = USB_CLASS_CDC,
            .bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
            .bFunctionProtocol = USB_PROTO_NONE,
            .iFunction = USB_STR_COM_PORT,
        },
    .interface_comm =
        {
            .bLength = sizeof(struct usb_interface_descriptor),
            .bDescriptorType = USB_DTYPE_INTERFACE,
            .bInterfaceNumber = USB_INTF_CDC_COMM,
            .bAlternateSetting = 0,
            .bNumEndpoints = 1,
            .bInterfaceClass = USB_CLASS_CDC,
            .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
            .bInterfaceProtocol = USB_PROTO_NONE,
            .iInterface = 0,
        },

    .cdc_header =
        {
            .bFunctionLength = sizeof(struct usb_cdc_header_desc),
            .bDescriptorType = USB_DTYPE_CS_INTERFACE,
            .bDescriptorSubType = USB_DTYPE_CDC_HEADER,
            .bcdCDC = VERSION_BCD(1, 1, 0),
        },

    .cdc_acm =
        {
            .bFunctionLength = sizeof(struct usb_cdc_call_mgmt_desc),
            .bDescriptorType = USB_DTYPE_CS_INTERFACE,
            .bDescriptorSubType = USB_DTYPE_CDC_CALL_MANAGEMENT,
            // .bmCapabilities = USB_CDC_CAP_LINE | USB_CDC_CAP_BRK,
            .bmCapabilities = 0,
        },

    .cdc_call_mgmt =
        {
            .bFunctionLength = sizeof(struct usb_cdc_acm_desc),
            .bDescriptorType = USB_DTYPE_CS_INTERFACE,
            .bDescriptorSubType = USB_DTYPE_CDC_ACM,
            .bmCapabilities = USB_CDC_CALL_MGMT_CAP_DATA_INTF,
            // .bDataInterface = USB_INTF_CDC_DATA,
        },

    .cdc_union =
        {
            .bFunctionLength = sizeof(struct usb_cdc_union_desc),
            .bDescriptorType = USB_DTYPE_CS_INTERFACE,
            .bDescriptorSubType = USB_DTYPE_CDC_UNION,
            .bMasterInterface0 = USB_INTF_CDC_COMM,
            .bSlaveInterface0 = USB_INTF_CDC_DATA,
        },

    .ep_comm =
        {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = HID_EP_IN | DAP_CDC_EP_COMM,
            .bmAttributes = USB_EPTYPE_INTERRUPT,
            .wMaxPacketSize = DAP_CDC_COMM_EP_SIZE,
            .bInterval = DAP_CDC_COMM_INTERVAL,
        },

    .interface_data =
        {
            .bLength = sizeof(struct usb_interface_descriptor),
            .bDescriptorType = USB_DTYPE_INTERFACE,
            .bInterfaceNumber = USB_INTF_CDC_DATA,
            .bAlternateSetting = 0,
            .bNumEndpoints = 2,
            .bInterfaceClass = USB_CLASS_CDC_DATA,
            .bInterfaceSubClass = USB_SUBCLASS_NONE,
            .bInterfaceProtocol = USB_PROTO_NONE,
            .iInterface = NO_DESCRIPTOR,
        },

    .ep_in =
        {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = HID_EP_IN | DAP_CDC_EP_SEND,
            .bmAttributes = USB_EPTYPE_BULK,
            .wMaxPacketSize = DAP_CDC_EP_SIZE,
            .bInterval = DAP_CDC_INTERVAL,
        },

    .ep_out =
        {
            .bLength = sizeof(struct usb_endpoint_descriptor),
            .bDescriptorType = USB_DTYPE_ENDPOINT,
            .bEndpointAddress = HID_EP_OUT | DAP_CDC_EP_RECV,
            .bmAttributes = USB_EPTYPE_BULK,
            .wMaxPacketSize = DAP_CDC_EP_SIZE,
            .bInterval = DAP_CDC_INTERVAL,
        },
};

// WinUSB
#include "usb_winusb.h"

typedef struct USB_PACK {
    usb_binary_object_store_descriptor_t bos;
    usb_winusb_capability_descriptor_t winusb;
} usb_bos_hierarchy_t;

typedef struct USB_PACK {
    usb_winusb_subset_header_function_t header;
    usb_winusb_feature_compatble_id_t comp_id;
    usb_winusb_feature_reg_property_guids_t property;
} usb_msos_descriptor_subset_t;

typedef struct USB_PACK {
    usb_winusb_set_header_descriptor_t header;
    usb_msos_descriptor_subset_t subset;
} usb_msos_descriptor_set_t;

#define USB_DTYPE_BINARY_OBJECT_STORE 15
#define USB_DTYPE_DEVICE_CAPABILITY_DESCRIPTOR 16
#define USB_DC_TYPE_PLATFORM 5

const usb_bos_hierarchy_t usb_bos_hierarchy = {
    .bos =
        {
            .bLength = sizeof(usb_binary_object_store_descriptor_t),
            .bDescriptorType = USB_DTYPE_BINARY_OBJECT_STORE,
            .wTotalLength = sizeof(usb_bos_hierarchy_t),
            .bNumDeviceCaps = 1,
        },
    .winusb =
        {
            .bLength = sizeof(usb_winusb_capability_descriptor_t),
            .bDescriptorType = USB_DTYPE_DEVICE_CAPABILITY_DESCRIPTOR,
            .bDevCapabilityType = USB_DC_TYPE_PLATFORM,
            .bReserved = 0,
            .PlatformCapabilityUUID = USB_WINUSB_PLATFORM_CAPABILITY_ID,
            .dwWindowsVersion = USB_WINUSB_WINDOWS_VERSION,
            .wMSOSDescriptorSetTotalLength = sizeof(usb_msos_descriptor_set_t),
            .bMS_VendorCode = USB_WINUSB_VENDOR_CODE,
            .bAltEnumCode = 0,
        },
};

const usb_msos_descriptor_set_t usb_msos_descriptor_set = {
    .header =
        {
            .wLength = sizeof(usb_winusb_set_header_descriptor_t),
            .wDescriptorType = USB_WINUSB_SET_HEADER_DESCRIPTOR,
            .dwWindowsVersion = USB_WINUSB_WINDOWS_VERSION,
            .wDescriptorSetTotalLength = sizeof(usb_msos_descriptor_set_t),
        },

    .subset =
        {
            .header =
                {
                    .wLength = sizeof(usb_winusb_subset_header_function_t),
                    .wDescriptorType = USB_WINUSB_SUBSET_HEADER_FUNCTION,
                    .bFirstInterface = USB_INTF_BULK,
                    .bReserved = 0,
                    .wSubsetLength = sizeof(usb_msos_descriptor_subset_t),
                },

            .comp_id =
                {
                    .wLength = sizeof(usb_winusb_feature_compatble_id_t),
                    .wDescriptorType = USB_WINUSB_FEATURE_COMPATBLE_ID,
                    .CompatibleID = "WINUSB\0\0",
                    .SubCompatibleID = {0},
                },

            .property =
                {
                    .wLength = sizeof(usb_winusb_feature_reg_property_guids_t),
                    .wDescriptorType = USB_WINUSB_FEATURE_REG_PROPERTY,
                    .wPropertyDataType = USB_WINUSB_PROPERTY_DATA_TYPE_MULTI_SZ,
                    .wPropertyNameLength =
                        sizeof(usb_msos_descriptor_set.subset.property.PropertyName),
                    .PropertyName = {'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0, 'I', 0,
                                     'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0,
                                     'e', 0, 'G', 0, 'U', 0, 'I', 0, 'D', 0, 's', 0, 0,   0},
                    .wPropertyDataLength =
                        sizeof(usb_msos_descriptor_set.subset.property.PropertyData),
                    .PropertyData = {'{', 0, 'C', 0, 'D', 0, 'B', 0, '3', 0, 'B', 0, '5', 0,
                                     'A', 0, 'D', 0, '-', 0, '2', 0, '9', 0, '3', 0, 'B', 0,
                                     '-', 0, '4', 0, '6', 0, '6', 0, '3', 0, '-', 0, 'A', 0,
                                     'A', 0, '3', 0, '6', 0, '-', 0, '1', 0, 'A', 0, 'A', 0,
                                     'E', 0, '4', 0, '6', 0, '4', 0, '6', 0, '3', 0, '7', 0,
                                     '7', 0, '6', 0, '}', 0, 0,   0, 0,   0},
                },
        },
};

typedef struct {
    FuriSemaphore* semaphore_v1;
    FuriSemaphore* semaphore_v2;
    FuriSemaphore* semaphore_cdc;
    bool connected;
    usbd_device* usb_dev;
    DapStateCallback state_callback;
    DapRxCallback rx_callback_v1;
    DapRxCallback rx_callback_v2;
    DapRxCallback rx_callback_cdc;
    DapCDCControlLineCallback control_line_callback_cdc;
    DapCDCConfigCallback config_callback_cdc;
    void* context;
    void* context_cdc;
} DAPState;

static DAPState dap_state = {
    .semaphore_v1 = NULL,
    .semaphore_v2 = NULL,
    .semaphore_cdc = NULL,
    .connected = false,
    .usb_dev = NULL,
    .state_callback = NULL,
    .rx_callback_v1 = NULL,
    .rx_callback_v2 = NULL,
    .rx_callback_cdc = NULL,
    .control_line_callback_cdc = NULL,
    .config_callback_cdc = NULL,
    .context = NULL,
    .context_cdc = NULL,
};

static struct usb_cdc_line_coding cdc_config = {0};
static uint8_t cdc_ctrl_line_state = 0;

#ifdef DAP_USB_LOG
void furi_console_log_printf(const char* format, ...) _ATTRIBUTE((__format__(__printf__, 1, 2)));

void furi_console_log_printf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    furi_hal_console_puts(buffer);
    furi_hal_console_puts("\r\n");
    UNUSED(format);
}
#else
#define furi_console_log_printf(...)
#endif

int32_t dap_v1_usb_tx(uint8_t* buffer, uint8_t size) {
    if((dap_state.semaphore_v1 == NULL) || (dap_state.connected == false)) return 0;

    furi_check(furi_semaphore_acquire(dap_state.semaphore_v1, FuriWaitForever) == FuriStatusOk);

    if(dap_state.connected) {
        int32_t len = usbd_ep_write(dap_state.usb_dev, DAP_HID_EP_IN, buffer, size);
        furi_console_log_printf("v1 tx %ld", len);
        return len;
    } else {
        return 0;
    }
}

int32_t dap_v2_usb_tx(uint8_t* buffer, uint8_t size) {
    if((dap_state.semaphore_v2 == NULL) || (dap_state.connected == false)) return 0;

    furi_check(furi_semaphore_acquire(dap_state.semaphore_v2, FuriWaitForever) == FuriStatusOk);

    if(dap_state.connected) {
        int32_t len = usbd_ep_write(dap_state.usb_dev, DAP_HID_EP_BULK_IN, buffer, size);
        furi_console_log_printf("v2 tx %ld", len);
        return len;
    } else {
        return 0;
    }
}

int32_t dap_cdc_usb_tx(uint8_t* buffer, uint8_t size) {
    if((dap_state.semaphore_cdc == NULL) || (dap_state.connected == false)) return 0;

    furi_check(furi_semaphore_acquire(dap_state.semaphore_cdc, FuriWaitForever) == FuriStatusOk);

    if(dap_state.connected) {
        int32_t len = usbd_ep_write(dap_state.usb_dev, HID_EP_IN | DAP_CDC_EP_SEND, buffer, size);
        furi_console_log_printf("cdc tx %ld", len);
        return len;
    } else {
        return 0;
    }
}

void dap_v1_usb_set_rx_callback(DapRxCallback callback) {
    dap_state.rx_callback_v1 = callback;
}

void dap_v2_usb_set_rx_callback(DapRxCallback callback) {
    dap_state.rx_callback_v2 = callback;
}

void dap_cdc_usb_set_rx_callback(DapRxCallback callback) {
    dap_state.rx_callback_cdc = callback;
}

void dap_cdc_usb_set_control_line_callback(DapCDCControlLineCallback callback) {
    dap_state.control_line_callback_cdc = callback;
}

void dap_cdc_usb_set_config_callback(DapCDCConfigCallback callback) {
    dap_state.config_callback_cdc = callback;
}

void dap_cdc_usb_set_context(void* context) {
    dap_state.context_cdc = context;
}

void dap_common_usb_set_context(void* context) {
    dap_state.context = context;
}

void dap_common_usb_set_state_callback(DapStateCallback callback) {
    dap_state.state_callback = callback;
}

static void* dap_usb_alloc_string_descr(const char* str) {
    furi_assert(str);

    size_t len = strlen(str);
    size_t wlen = (len + 1) * sizeof(uint16_t);
    struct usb_string_descriptor* dev_str_desc = malloc(wlen);
    dev_str_desc->bLength = wlen;
    dev_str_desc->bDescriptorType = USB_DTYPE_STRING;
    for(size_t i = 0; i < len; i++) {
        dev_str_desc->wString[i] = str[i];
    }

    return dev_str_desc;
}

void dap_common_usb_alloc_name(const char* name) {
    dev_serial_descr = dap_usb_alloc_string_descr(name);
}

void dap_common_usb_free_name() {
    free(dev_serial_descr);
}

static void hid_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx);
static void hid_deinit(usbd_device* dev);
static void hid_on_wakeup(usbd_device* dev);
static void hid_on_suspend(usbd_device* dev);

static usbd_respond hid_ep_config(usbd_device* dev, uint8_t cfg);
static usbd_respond hid_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback);

FuriHalUsbInterface dap_v2_usb_hid = {
    .init = hid_init,
    .deinit = hid_deinit,
    .wakeup = hid_on_wakeup,
    .suspend = hid_on_suspend,
    .dev_descr = (struct usb_device_descriptor*)&hid_device_desc,
    .cfg_descr = (void*)&hid_cfg_desc,
};

static void hid_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx) {
    UNUSED(intf);
    UNUSED(ctx);

    dap_v2_usb_hid.str_manuf_descr = (void*)&dev_manuf_descr;
    dap_v2_usb_hid.str_prod_descr = (void*)&dev_prod_descr;
    dap_v2_usb_hid.str_serial_descr = (void*)dev_serial_descr;

    dap_state.usb_dev = dev;
    if(dap_state.semaphore_v1 == NULL) dap_state.semaphore_v1 = furi_semaphore_alloc(1, 1);
    if(dap_state.semaphore_v2 == NULL) dap_state.semaphore_v2 = furi_semaphore_alloc(1, 1);
    if(dap_state.semaphore_cdc == NULL) dap_state.semaphore_cdc = furi_semaphore_alloc(1, 1);

    usbd_reg_config(dev, hid_ep_config);
    usbd_reg_control(dev, hid_control);

    usbd_connect(dev, true);
}

static void hid_deinit(usbd_device* dev) {
    dap_state.usb_dev = NULL;

    furi_semaphore_free(dap_state.semaphore_v1);
    furi_semaphore_free(dap_state.semaphore_v2);
    furi_semaphore_free(dap_state.semaphore_cdc);
    dap_state.semaphore_v1 = NULL;
    dap_state.semaphore_v2 = NULL;
    dap_state.semaphore_cdc = NULL;

    usbd_reg_config(dev, NULL);
    usbd_reg_control(dev, NULL);
}

static void hid_on_wakeup(usbd_device* dev) {
    UNUSED(dev);
    if(!dap_state.connected) {
        dap_state.connected = true;
        if(dap_state.state_callback != NULL) {
            dap_state.state_callback(dap_state.connected, dap_state.context);
        }
    }
}

static void hid_on_suspend(usbd_device* dev) {
    UNUSED(dev);
    if(dap_state.connected) {
        dap_state.connected = false;
        if(dap_state.state_callback != NULL) {
            dap_state.state_callback(dap_state.connected, dap_state.context);
        }
    }
}

size_t dap_v1_usb_rx(uint8_t* buffer, size_t size) {
    size_t len = 0;

    if(dap_state.connected) {
        len = usbd_ep_read(dap_state.usb_dev, DAP_HID_EP_OUT, buffer, size);
    }

    return len;
}

size_t dap_v2_usb_rx(uint8_t* buffer, size_t size) {
    size_t len = 0;

    if(dap_state.connected) {
        len = usbd_ep_read(dap_state.usb_dev, DAP_HID_EP_BULK_OUT, buffer, size);
    }

    return len;
}

size_t dap_cdc_usb_rx(uint8_t* buffer, size_t size) {
    size_t len = 0;

    if(dap_state.connected) {
        len = usbd_ep_read(dap_state.usb_dev, HID_EP_OUT | DAP_CDC_EP_RECV, buffer, size);
    }

    return len;
}

static void hid_txrx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(ep);

    switch(event) {
    case usbd_evt_eptx:
        furi_semaphore_release(dap_state.semaphore_v1);
        furi_console_log_printf("hid tx complete");
        break;
    case usbd_evt_eprx:
        if(dap_state.rx_callback_v1 != NULL) {
            dap_state.rx_callback_v1(dap_state.context);
        }
        break;
    default:
        furi_console_log_printf("hid %d, %d", event, ep);
        break;
    }
}

static void hid_txrx_ep_bulk_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(ep);

    switch(event) {
    case usbd_evt_eptx:
        furi_semaphore_release(dap_state.semaphore_v2);
        furi_console_log_printf("bulk tx complete");
        break;
    case usbd_evt_eprx:
        if(dap_state.rx_callback_v2 != NULL) {
            dap_state.rx_callback_v2(dap_state.context);
        }
        break;
    default:
        furi_console_log_printf("bulk %d, %d", event, ep);
        break;
    }
}

static void cdc_txrx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
    UNUSED(dev);
    UNUSED(ep);

    switch(event) {
    case usbd_evt_eptx:
        furi_semaphore_release(dap_state.semaphore_cdc);
        furi_console_log_printf("cdc tx complete");
        break;
    case usbd_evt_eprx:
        if(dap_state.rx_callback_cdc != NULL) {
            dap_state.rx_callback_cdc(dap_state.context_cdc);
        }
        break;
    default:
        furi_console_log_printf("cdc %d, %d", event, ep);
        break;
    }
}

static usbd_respond hid_ep_config(usbd_device* dev, uint8_t cfg) {
    switch(cfg) {
    case EP_CFG_DECONFIGURE:
        usbd_ep_deconfig(dev, DAP_HID_EP_OUT);
        usbd_ep_deconfig(dev, DAP_HID_EP_IN);
        usbd_ep_deconfig(dev, DAP_HID_EP_BULK_IN);
        usbd_ep_deconfig(dev, DAP_HID_EP_BULK_OUT);
        usbd_ep_deconfig(dev, HID_EP_IN | DAP_CDC_EP_COMM);
        usbd_ep_deconfig(dev, HID_EP_IN | DAP_CDC_EP_SEND);
        usbd_ep_deconfig(dev, HID_EP_OUT | DAP_CDC_EP_RECV);
        usbd_reg_endpoint(dev, DAP_HID_EP_OUT, NULL);
        usbd_reg_endpoint(dev, DAP_HID_EP_IN, NULL);
        usbd_reg_endpoint(dev, DAP_HID_EP_BULK_IN, NULL);
        usbd_reg_endpoint(dev, DAP_HID_EP_BULK_OUT, NULL);
        usbd_reg_endpoint(dev, HID_EP_IN | DAP_CDC_EP_SEND, 0);
        usbd_reg_endpoint(dev, HID_EP_OUT | DAP_CDC_EP_RECV, 0);
        return usbd_ack;
    case EP_CFG_CONFIGURE:
        usbd_ep_config(dev, DAP_HID_EP_IN, USB_EPTYPE_INTERRUPT, DAP_HID_EP_SIZE);
        usbd_ep_config(dev, DAP_HID_EP_OUT, USB_EPTYPE_INTERRUPT, DAP_HID_EP_SIZE);
        usbd_ep_config(dev, DAP_HID_EP_BULK_OUT, USB_EPTYPE_BULK, DAP_HID_EP_SIZE);
        usbd_ep_config(dev, DAP_HID_EP_BULK_IN, USB_EPTYPE_BULK, DAP_HID_EP_SIZE);
        usbd_ep_config(dev, HID_EP_OUT | DAP_CDC_EP_RECV, USB_EPTYPE_BULK, DAP_CDC_EP_SIZE);
        usbd_ep_config(dev, HID_EP_IN | DAP_CDC_EP_SEND, USB_EPTYPE_BULK, DAP_CDC_EP_SIZE);
        usbd_ep_config(dev, HID_EP_IN | DAP_CDC_EP_COMM, USB_EPTYPE_INTERRUPT, DAP_CDC_EP_SIZE);
        usbd_reg_endpoint(dev, DAP_HID_EP_IN, hid_txrx_ep_callback);
        usbd_reg_endpoint(dev, DAP_HID_EP_OUT, hid_txrx_ep_callback);
        usbd_reg_endpoint(dev, DAP_HID_EP_BULK_OUT, hid_txrx_ep_bulk_callback);
        usbd_reg_endpoint(dev, DAP_HID_EP_BULK_IN, hid_txrx_ep_bulk_callback);
        usbd_reg_endpoint(dev, HID_EP_OUT | DAP_CDC_EP_RECV, cdc_txrx_ep_callback);
        usbd_reg_endpoint(dev, HID_EP_IN | DAP_CDC_EP_SEND, cdc_txrx_ep_callback);
        // usbd_ep_write(dev, DAP_HID_EP_IN, NULL, 0);
        // usbd_ep_write(dev, DAP_HID_EP_BULK_IN, NULL, 0);
        // usbd_ep_write(dev, HID_EP_IN | DAP_CDC_EP_SEND, NULL, 0);
        return usbd_ack;
    default:
        return usbd_fail;
    }
}

#ifdef DAP_USB_LOG
static void dump_request_type(uint8_t type) {
    switch(type & USB_REQ_DIRECTION) {
    case USB_REQ_HOSTTODEV:
        furi_hal_console_puts("host to dev, ");
        break;
    case USB_REQ_DEVTOHOST:
        furi_hal_console_puts("dev to host, ");
        break;
    }

    switch(type & USB_REQ_TYPE) {
    case USB_REQ_STANDARD:
        furi_hal_console_puts("standard, ");
        break;
    case USB_REQ_CLASS:
        furi_hal_console_puts("class, ");
        break;
    case USB_REQ_VENDOR:
        furi_hal_console_puts("vendor, ");
        break;
    }

    switch(type & USB_REQ_RECIPIENT) {
    case USB_REQ_DEVICE:
        furi_hal_console_puts("device");
        break;
    case USB_REQ_INTERFACE:
        furi_hal_console_puts("interface");
        break;
    case USB_REQ_ENDPOINT:
        furi_hal_console_puts("endpoint");
        break;
    case USB_REQ_OTHER:
        furi_hal_console_puts("other");
        break;
    }

    furi_hal_console_puts("\r\n");
}
#else
#define dump_request_type(...)
#endif

static usbd_respond hid_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback) {
    UNUSED(callback);

    dump_request_type(req->bmRequestType);
    furi_console_log_printf(
        "control: RT %02x, R %02x, V %04x, I %04x, L %04x",
        req->bmRequestType,
        req->bRequest,
        req->wValue,
        req->wIndex,
        req->wLength);

    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE | USB_REQ_DIRECTION) & req->bmRequestType) ==
       (USB_REQ_STANDARD | USB_REQ_VENDOR | USB_REQ_DEVTOHOST)) {
        // vendor request, device to host
        furi_console_log_printf("vendor request");
        if(USB_WINUSB_VENDOR_CODE == req->bRequest) {
            // WINUSB request
            if(USB_WINUSB_DESCRIPTOR_INDEX == req->wIndex) {
                furi_console_log_printf("WINUSB descriptor");
                uint16_t length = req->wLength;
                if(length > sizeof(usb_msos_descriptor_set_t)) {
                    length = sizeof(usb_msos_descriptor_set_t);
                }

                dev->status.data_ptr = (uint8_t*)&usb_msos_descriptor_set;
                dev->status.data_count = length;
                return usbd_ack;
            }
        }
    }

    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
       (USB_REQ_STANDARD | USB_REQ_DEVICE)) {
        // device request
        if(req->bRequest == USB_STD_GET_DESCRIPTOR) {
            const uint8_t dtype = req->wValue >> 8;
            const uint8_t dnumber = req->wValue & 0xFF;
            // get string descriptor
            if(USB_DTYPE_STRING == dtype) {
                if(dnumber == USB_STR_CMSIS_DAP_V1) {
                    furi_console_log_printf("str CMSIS-DAP v1");
                    dev->status.data_ptr = (uint8_t*)&dev_dap_v1_descr;
                    dev->status.data_count = dev_dap_v1_descr.bLength;
                    return usbd_ack;
                } else if(dnumber == USB_STR_CMSIS_DAP_V2) {
                    furi_console_log_printf("str CMSIS-DAP v2");
                    dev->status.data_ptr = (uint8_t*)&dev_dap_v2_descr;
                    dev->status.data_count = dev_dap_v2_descr.bLength;
                    return usbd_ack;
                } else if(dnumber == USB_STR_COM_PORT) {
                    furi_console_log_printf("str COM port");
                    dev->status.data_ptr = (uint8_t*)&dev_com_descr;
                    dev->status.data_count = dev_com_descr.bLength;
                    return usbd_ack;
                }
            } else if(USB_DTYPE_BINARY_OBJECT_STORE == dtype) {
                furi_console_log_printf("BOS descriptor");
                uint16_t length = req->wLength;
                if(length > sizeof(usb_bos_hierarchy_t)) {
                    length = sizeof(usb_bos_hierarchy_t);
                }
                dev->status.data_ptr = (uint8_t*)&usb_bos_hierarchy;
                dev->status.data_count = length;
                return usbd_ack;
            }
        }
    }

    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
           (USB_REQ_INTERFACE | USB_REQ_CLASS) &&
       req->wIndex == 0) {
        // class request
        switch(req->bRequest) {
        // get hid descriptor
        case USB_HID_GETREPORT:
            furi_console_log_printf("get report");
            return usbd_fail;
        // set hid idle
        case USB_HID_SETIDLE:
            furi_console_log_printf("set idle");
            return usbd_ack;
        default:
            break;
        }
    }

    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
           (USB_REQ_INTERFACE | USB_REQ_CLASS) &&
       req->wIndex == 2) {
        // class request
        switch(req->bRequest) {
        // control line state
        case USB_CDC_SET_CONTROL_LINE_STATE:
            furi_console_log_printf("set control line state");
            cdc_ctrl_line_state = req->wValue;
            if(dap_state.control_line_callback_cdc != NULL) {
                dap_state.control_line_callback_cdc(cdc_ctrl_line_state, dap_state.context_cdc);
            }
            return usbd_ack;
        // set cdc line coding
        case USB_CDC_SET_LINE_CODING:
            furi_console_log_printf("set line coding");
            memcpy(&cdc_config, req->data, sizeof(cdc_config));
            if(dap_state.config_callback_cdc != NULL) {
                dap_state.config_callback_cdc(&cdc_config, dap_state.context_cdc);
            }
            return usbd_ack;
        // get cdc line coding
        case USB_CDC_GET_LINE_CODING:
            furi_console_log_printf("get line coding");
            dev->status.data_ptr = &cdc_config;
            dev->status.data_count = sizeof(cdc_config);
            return usbd_ack;
        default:
            break;
        }
    }

    if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
           (USB_REQ_INTERFACE | USB_REQ_STANDARD) &&
       req->wIndex == 0 && req->bRequest == USB_STD_GET_DESCRIPTOR) {
        // standard request
        switch(req->wValue >> 8) {
        // get hid descriptor
        case USB_DTYPE_HID:
            furi_console_log_printf("get hid descriptor");
            dev->status.data_ptr = (uint8_t*)&(hid_cfg_desc.hid);
            dev->status.data_count = sizeof(hid_cfg_desc.hid);
            return usbd_ack;
        // get hid report descriptor
        case USB_DTYPE_HID_REPORT:
            furi_console_log_printf("get hid report descriptor");
            dev->status.data_ptr = (uint8_t*)hid_report_desc;
            dev->status.data_count = sizeof(hid_report_desc);
            return usbd_ack;
        default:
            break;
        }
    }

    return usbd_fail;
}
