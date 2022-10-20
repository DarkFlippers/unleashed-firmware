#pragma once
#include <stdint.h>

/*- Definitions -------------------------------------------------------------*/

#define USB_PACK __attribute__((packed))

#define USB_WINUSB_VENDOR_CODE 0x20

#define USB_WINUSB_WINDOWS_VERSION 0x06030000 // Windows 8.1

#define USB_WINUSB_PLATFORM_CAPABILITY_ID                                                         \
    {                                                                                             \
        0xdf, 0x60, 0xdd, 0xd8, 0x89, 0x45, 0xc7, 0x4c, 0x9c, 0xd2, 0x65, 0x9d, 0x9e, 0x64, 0x8a, \
            0x9f                                                                                  \
    }

enum // WinUSB Microsoft OS 2.0 descriptor request codes
{
    USB_WINUSB_DESCRIPTOR_INDEX = 0x07,
    USB_WINUSB_SET_ALT_ENUMERATION = 0x08,
};

enum // wDescriptorType
{
    USB_WINUSB_SET_HEADER_DESCRIPTOR = 0x00,
    USB_WINUSB_SUBSET_HEADER_CONFIGURATION = 0x01,
    USB_WINUSB_SUBSET_HEADER_FUNCTION = 0x02,
    USB_WINUSB_FEATURE_COMPATBLE_ID = 0x03,
    USB_WINUSB_FEATURE_REG_PROPERTY = 0x04,
    USB_WINUSB_FEATURE_MIN_RESUME_TIME = 0x05,
    USB_WINUSB_FEATURE_MODEL_ID = 0x06,
    USB_WINUSB_FEATURE_CCGP_DEVICE = 0x07,
    USB_WINUSB_FEATURE_VENDOR_REVISION = 0x08,
};

enum // wPropertyDataType
{
    USB_WINUSB_PROPERTY_DATA_TYPE_SZ = 1,
    USB_WINUSB_PROPERTY_DATA_TYPE_EXPAND_SZ = 2,
    USB_WINUSB_PROPERTY_DATA_TYPE_BINARY = 3,
    USB_WINUSB_PROPERTY_DATA_TYPE_DWORD_LITTLE_ENDIAN = 4,
    USB_WINUSB_PROPERTY_DATA_TYPE_DWORD_BIG_ENDIAN = 5,
    USB_WINUSB_PROPERTY_DATA_TYPE_LINK = 6,
    USB_WINUSB_PROPERTY_DATA_TYPE_MULTI_SZ = 7,
};

/*- Types BOS -------------------------------------------------------------------*/

typedef struct USB_PACK {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumDeviceCaps;
} usb_binary_object_store_descriptor_t;

/*- Types WinUSB -------------------------------------------------------------------*/

typedef struct USB_PACK {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDevCapabilityType;
    uint8_t bReserved;
    uint8_t PlatformCapabilityUUID[16];
    uint32_t dwWindowsVersion;
    uint16_t wMSOSDescriptorSetTotalLength;
    uint8_t bMS_VendorCode;
    uint8_t bAltEnumCode;
} usb_winusb_capability_descriptor_t;

typedef struct USB_PACK {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint32_t dwWindowsVersion;
    uint16_t wDescriptorSetTotalLength;
} usb_winusb_set_header_descriptor_t;

typedef struct USB_PACK {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint8_t bConfigurationValue;
    uint8_t bReserved;
    uint16_t wTotalLength;
} usb_winusb_subset_header_configuration_t;

typedef struct USB_PACK {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint8_t bFirstInterface;
    uint8_t bReserved;
    uint16_t wSubsetLength;
} usb_winusb_subset_header_function_t;

typedef struct USB_PACK {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint8_t CompatibleID[8];
    uint8_t SubCompatibleID[8];
} usb_winusb_feature_compatble_id_t;

typedef struct USB_PACK {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint16_t wPropertyDataType;
    //uint16_t  wPropertyNameLength;
    //uint8_t   PropertyName[...];
    //uint16_t  wPropertyDataLength
    //uint8_t   PropertyData[...];
} usb_winusb_feature_reg_property_t;

typedef struct USB_PACK {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint16_t wPropertyDataType;
    uint16_t wPropertyNameLength;
    uint8_t PropertyName[42];
    uint16_t wPropertyDataLength;
    uint8_t PropertyData[80];
} usb_winusb_feature_reg_property_guids_t;

typedef struct USB_PACK {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint8_t bResumeRecoveryTime;
    uint8_t bResumeSignalingTime;
} usb_winusb_feature_min_resume_time_t;

typedef struct USB_PACK {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint8_t ModelID[16];
} usb_winusb_feature_model_id_t;

typedef struct USB_PACK {
    uint16_t wLength;
    uint16_t wDescriptorType;
} usb_winusb_feature_ccgp_device_t;

typedef struct USB_PACK {
    uint16_t wLength;
    uint16_t wDescriptorType;
    uint16_t VendorRevision;
} usb_winusb_feature_vendor_revision_t;