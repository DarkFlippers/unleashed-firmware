#include "dev_info_service.h"
#include "app_common.h"
#include "ble.h"

#include <furi.h>

#define DEV_INFO_SERVICE_TAG "dev info service"

typedef struct {
    uint16_t service_handle;
    uint16_t man_name_char_handle;
    uint16_t serial_num_char_handle;
    uint16_t firmware_rev_char_handle;
    uint16_t software_rev_char_handle;
} DevInfoSvc;

bool dev_info_service_init() {
    tBleStatus status;
    DevInfoSvc dev_info_svc;

    // Add Device Information Service
    uint16_t uuid = DEVICE_INFORMATION_SERVICE_UUID;
    status = aci_gatt_add_service(UUID_TYPE_16, (Service_UUID_t*)&uuid, PRIMARY_SERVICE, 9, &dev_info_svc.service_handle);
    if(status) {
        FURI_LOG_E(DEV_INFO_SERVICE_TAG, "Failed to add Device Information Service: %d", status);
    }

    // Add characteristics
    uuid = MANUFACTURER_NAME_UUID;
    status = aci_gatt_add_char(dev_info_svc.service_handle,
                                UUID_TYPE_16,
                                (Char_UUID_t*)&uuid,
                                strlen(DEV_INFO_MANUFACTURER_NAME),
                                CHAR_PROP_READ,
                                ATTR_PERMISSION_NONE,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_CONSTANT,
                                &dev_info_svc.man_name_char_handle);
    if(status) {
        FURI_LOG_E(DEV_INFO_SERVICE_TAG, "Failed to add manufacturer name char: %d", status);

    }
    uuid = SERIAL_NUMBER_UUID;
    status = aci_gatt_add_char(dev_info_svc.service_handle,
                                UUID_TYPE_16,
                                (Char_UUID_t*)&uuid,
                                strlen(DEV_INFO_SERIAL_NUMBER),
                                CHAR_PROP_READ,
                                ATTR_PERMISSION_NONE,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_CONSTANT,
                                &dev_info_svc.serial_num_char_handle);
    if(status) {
        FURI_LOG_E(DEV_INFO_SERVICE_TAG, "Failed to add serial number char: %d", status);
    }
    uuid = FIRMWARE_REVISION_UUID;
    status = aci_gatt_add_char(dev_info_svc.service_handle,
                                UUID_TYPE_16,
                                (Char_UUID_t*)&uuid,
                                strlen(DEV_INFO_FIRMWARE_REVISION_NUMBER),
                                CHAR_PROP_READ,
                                ATTR_PERMISSION_NONE,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_CONSTANT,
                                &dev_info_svc.firmware_rev_char_handle);
    if(status) {
        FURI_LOG_E(DEV_INFO_SERVICE_TAG, "Failed to add firmware revision char: %d", status);
    }
    uuid = SOFTWARE_REVISION_UUID;
    status = aci_gatt_add_char(dev_info_svc.service_handle,
                                UUID_TYPE_16,
                                (Char_UUID_t*)&uuid,
                                strlen(DEV_INFO_SOFTWARE_REVISION_NUMBER),
                                CHAR_PROP_READ,
                                ATTR_PERMISSION_NONE,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_CONSTANT,
                                &dev_info_svc.software_rev_char_handle);
    if(status) {
        FURI_LOG_E(DEV_INFO_SERVICE_TAG, "Failed to add software revision char: %d", status);
    }

    // Update characteristics
    status = aci_gatt_update_char_value(dev_info_svc.service_handle,
                                        dev_info_svc.man_name_char_handle,
                                        0,
                                        strlen(DEV_INFO_MANUFACTURER_NAME),
                                        (uint8_t*)DEV_INFO_MANUFACTURER_NAME);
    if(status) {
        FURI_LOG_E(DEV_INFO_SERVICE_TAG, "Failed to update manufacturer name char: %d", status);
    }
    status = aci_gatt_update_char_value(dev_info_svc.service_handle,
                                        dev_info_svc.serial_num_char_handle,
                                        0,
                                        strlen(DEV_INFO_SERIAL_NUMBER),
                                        (uint8_t*)DEV_INFO_SERIAL_NUMBER);
    if(status) {
        FURI_LOG_E(DEV_INFO_SERVICE_TAG, "Failed to update serial number char: %d", status);
    }
    status = aci_gatt_update_char_value(dev_info_svc.service_handle,
                                        dev_info_svc.firmware_rev_char_handle,
                                        0,
                                        strlen(DEV_INFO_FIRMWARE_REVISION_NUMBER),
                                        (uint8_t*)DEV_INFO_FIRMWARE_REVISION_NUMBER);
    if(status) {
        FURI_LOG_E(DEV_INFO_SERVICE_TAG, "Failed to update firmware revision char: %d", status);
    }
    status = aci_gatt_update_char_value(dev_info_svc.service_handle,
                                        dev_info_svc.software_rev_char_handle,
                                        0,
                                        strlen(DEV_INFO_SOFTWARE_REVISION_NUMBER),
                                        (uint8_t*)DEV_INFO_SOFTWARE_REVISION_NUMBER);
    if(status) {
        FURI_LOG_E(DEV_INFO_SERVICE_TAG, "Failed to update software revision char: %d", status);
    }
    return status != BLE_STATUS_SUCCESS;
}
