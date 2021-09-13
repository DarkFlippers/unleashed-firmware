#include "dev_info_service.h"
#include "app_common.h"
#include "ble.h"

#include <furi.h>

#define DEV_INFO_SVC_TAG "dev info service"

typedef struct {
    uint16_t service_handle;
    uint16_t man_name_char_handle;
    uint16_t serial_num_char_handle;
    uint16_t firmware_rev_char_handle;
    uint16_t software_rev_char_handle;
} DevInfoSvc;

static DevInfoSvc* dev_info_svc = NULL;

static const char dev_info_man_name[] = "Flipper Devices Inc.";
static const char dev_info_serial_num[] = "1.0";
static const char dev_info_firmware_rev_num[] = TARGET;
static const char dev_info_software_rev_num[] = GIT_COMMIT " " GIT_BRANCH " " GIT_BRANCH_NUM " " BUILD_DATE;

void dev_info_svc_start() {
    dev_info_svc = furi_alloc(sizeof(DevInfoSvc));
    tBleStatus status;

    // Add Device Information Service
    uint16_t uuid = DEVICE_INFORMATION_SERVICE_UUID;
    status = aci_gatt_add_service(UUID_TYPE_16, (Service_UUID_t*)&uuid, PRIMARY_SERVICE, 9, &dev_info_svc->service_handle);
    if(status) {
        FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to add Device Information Service: %d", status);
    }

    // Add characteristics
    uuid = MANUFACTURER_NAME_UUID;
    status = aci_gatt_add_char(dev_info_svc->service_handle,
                                UUID_TYPE_16,
                                (Char_UUID_t*)&uuid,
                                strlen(dev_info_man_name),
                                CHAR_PROP_READ,
                                ATTR_PERMISSION_NONE,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_CONSTANT,
                                &dev_info_svc->man_name_char_handle);
    if(status) {
        FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to add manufacturer name char: %d", status);
    }
    uuid = SERIAL_NUMBER_UUID;
    status = aci_gatt_add_char(dev_info_svc->service_handle,
                                UUID_TYPE_16,
                                (Char_UUID_t*)&uuid,
                                strlen(dev_info_serial_num),
                                CHAR_PROP_READ,
                                ATTR_PERMISSION_NONE,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_CONSTANT,
                                &dev_info_svc->serial_num_char_handle);
    if(status) {
        FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to add serial number char: %d", status);
    }
    uuid = FIRMWARE_REVISION_UUID;
    status = aci_gatt_add_char(dev_info_svc->service_handle,
                                UUID_TYPE_16,
                                (Char_UUID_t*)&uuid,
                                strlen(dev_info_firmware_rev_num),
                                CHAR_PROP_READ,
                                ATTR_PERMISSION_NONE,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_CONSTANT,
                                &dev_info_svc->firmware_rev_char_handle);
    if(status) {
        FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to add firmware revision char: %d", status);
    }
    uuid = SOFTWARE_REVISION_UUID;
    status = aci_gatt_add_char(dev_info_svc->service_handle,
                                UUID_TYPE_16,
                                (Char_UUID_t*)&uuid,
                                strlen(dev_info_software_rev_num),
                                CHAR_PROP_READ,
                                ATTR_PERMISSION_NONE,
                                GATT_DONT_NOTIFY_EVENTS,
                                10,
                                CHAR_VALUE_LEN_CONSTANT,
                                &dev_info_svc->software_rev_char_handle);
    if(status) {
        FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to add software revision char: %d", status);
    }

    // Update characteristics
    status = aci_gatt_update_char_value(dev_info_svc->service_handle,
                                        dev_info_svc->man_name_char_handle,
                                        0,
                                        strlen(dev_info_man_name),
                                        (uint8_t*)dev_info_man_name);
    if(status) {
        FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to update manufacturer name char: %d", status);
    }
    status = aci_gatt_update_char_value(dev_info_svc->service_handle,
                                        dev_info_svc->serial_num_char_handle,
                                        0,
                                        strlen(dev_info_serial_num),
                                        (uint8_t*)dev_info_serial_num);
    if(status) {
        FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to update serial number char: %d", status);
    }
    status = aci_gatt_update_char_value(dev_info_svc->service_handle,
                                        dev_info_svc->firmware_rev_char_handle,
                                        0,
                                        strlen(dev_info_firmware_rev_num),
                                        (uint8_t*)dev_info_firmware_rev_num);
    if(status) {
        FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to update firmware revision char: %d", status);
    }
    status = aci_gatt_update_char_value(dev_info_svc->service_handle,
                                        dev_info_svc->software_rev_char_handle,
                                        0,
                                        strlen(dev_info_software_rev_num),
                                        (uint8_t*)dev_info_software_rev_num);
    if(status) {
        FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to update software revision char: %d", status);
    }
}

void dev_info_svc_stop() {
    tBleStatus status;
    if(dev_info_svc) {
        // Delete service characteristics
        status = aci_gatt_del_char(dev_info_svc->service_handle, dev_info_svc->man_name_char_handle);
        if(status) {
            FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to delete manufacturer name char: %d", status);
        }
        status = aci_gatt_del_char(dev_info_svc->service_handle, dev_info_svc->serial_num_char_handle);
        if(status) {
            FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to delete serial number char: %d", status);
        }
        status = aci_gatt_del_char(dev_info_svc->service_handle, dev_info_svc->firmware_rev_char_handle);
        if(status) {
            FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to delete firmware revision char: %d", status);
        }
        status = aci_gatt_del_char(dev_info_svc->service_handle, dev_info_svc->software_rev_char_handle);
        if(status) {
            FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to delete software revision char: %d", status);
        }
        // Delete service
        status = aci_gatt_del_service(dev_info_svc->service_handle);
        if(status) {
            FURI_LOG_E(DEV_INFO_SVC_TAG, "Failed to delete device info service: %d", status);
        }
        free(dev_info_svc);
        dev_info_svc = NULL;
    }
}
