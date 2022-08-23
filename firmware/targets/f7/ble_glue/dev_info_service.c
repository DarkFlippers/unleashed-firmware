#include "dev_info_service.h"
#include "app_common.h"
#include <ble/ble.h>

#include <furi.h>
#include <m-string.h>
#include <protobuf_version.h>
#include <lib/toolbox/version.h>

#define TAG "BtDevInfoSvc"

typedef struct {
    uint16_t service_handle;
    uint16_t man_name_char_handle;
    uint16_t serial_num_char_handle;
    uint16_t firmware_rev_char_handle;
    uint16_t software_rev_char_handle;
    uint16_t rpc_version_char_handle;
    string_t version_string;
    char hardware_revision[4];
} DevInfoSvc;

static DevInfoSvc* dev_info_svc = NULL;

static const char dev_info_man_name[] = "Flipper Devices Inc.";
static const char dev_info_serial_num[] = "1.0";
static const char dev_info_rpc_version[] = TOSTRING(PROTOBUF_MAJOR_VERSION.PROTOBUF_MINOR_VERSION);

static const uint8_t dev_info_rpc_version_uuid[] =
    {0x33, 0xa9, 0xb5, 0x3e, 0x87, 0x5d, 0x1a, 0x8e, 0xc8, 0x47, 0x5e, 0xae, 0x6d, 0x66, 0xf6, 0x03};

void dev_info_svc_start() {
    dev_info_svc = malloc(sizeof(DevInfoSvc));
    string_init_printf(
        dev_info_svc->version_string,
        "%s %s %s %s",
        version_get_githash(NULL),
        version_get_gitbranch(NULL),
        version_get_gitbranchnum(NULL),
        version_get_builddate(NULL));
    snprintf(
        dev_info_svc->hardware_revision,
        sizeof(dev_info_svc->hardware_revision),
        "%d",
        version_get_target(NULL));
    tBleStatus status;

    // Add Device Information Service
    uint16_t uuid = DEVICE_INFORMATION_SERVICE_UUID;
    status = aci_gatt_add_service(
        UUID_TYPE_16, (Service_UUID_t*)&uuid, PRIMARY_SERVICE, 11, &dev_info_svc->service_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add Device Information Service: %d", status);
    }

    // Add characteristics
    uuid = MANUFACTURER_NAME_UUID;
    status = aci_gatt_add_char(
        dev_info_svc->service_handle,
        UUID_TYPE_16,
        (Char_UUID_t*)&uuid,
        strlen(dev_info_man_name),
        CHAR_PROP_READ,
        ATTR_PERMISSION_AUTHEN_READ,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &dev_info_svc->man_name_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add manufacturer name char: %d", status);
    }
    uuid = SERIAL_NUMBER_UUID;
    status = aci_gatt_add_char(
        dev_info_svc->service_handle,
        UUID_TYPE_16,
        (Char_UUID_t*)&uuid,
        strlen(dev_info_serial_num),
        CHAR_PROP_READ,
        ATTR_PERMISSION_AUTHEN_READ,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &dev_info_svc->serial_num_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add serial number char: %d", status);
    }
    uuid = FIRMWARE_REVISION_UUID;
    status = aci_gatt_add_char(
        dev_info_svc->service_handle,
        UUID_TYPE_16,
        (Char_UUID_t*)&uuid,
        strlen(dev_info_svc->hardware_revision),
        CHAR_PROP_READ,
        ATTR_PERMISSION_AUTHEN_READ,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &dev_info_svc->firmware_rev_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add firmware revision char: %d", status);
    }
    uuid = SOFTWARE_REVISION_UUID;
    status = aci_gatt_add_char(
        dev_info_svc->service_handle,
        UUID_TYPE_16,
        (Char_UUID_t*)&uuid,
        string_size(dev_info_svc->version_string),
        CHAR_PROP_READ,
        ATTR_PERMISSION_AUTHEN_READ,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &dev_info_svc->software_rev_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add software revision char: %d", status);
    }
    status = aci_gatt_add_char(
        dev_info_svc->service_handle,
        UUID_TYPE_128,
        (const Char_UUID_t*)dev_info_rpc_version_uuid,
        strlen(dev_info_rpc_version),
        CHAR_PROP_READ,
        ATTR_PERMISSION_AUTHEN_READ,
        GATT_DONT_NOTIFY_EVENTS,
        10,
        CHAR_VALUE_LEN_CONSTANT,
        &dev_info_svc->rpc_version_char_handle);
    if(status) {
        FURI_LOG_E(TAG, "Failed to add rpc version characteristic: %d", status);
    }

    // Update characteristics
    status = aci_gatt_update_char_value(
        dev_info_svc->service_handle,
        dev_info_svc->man_name_char_handle,
        0,
        strlen(dev_info_man_name),
        (uint8_t*)dev_info_man_name);
    if(status) {
        FURI_LOG_E(TAG, "Failed to update manufacturer name char: %d", status);
    }
    status = aci_gatt_update_char_value(
        dev_info_svc->service_handle,
        dev_info_svc->serial_num_char_handle,
        0,
        strlen(dev_info_serial_num),
        (uint8_t*)dev_info_serial_num);
    if(status) {
        FURI_LOG_E(TAG, "Failed to update serial number char: %d", status);
    }
    status = aci_gatt_update_char_value(
        dev_info_svc->service_handle,
        dev_info_svc->firmware_rev_char_handle,
        0,
        strlen(dev_info_svc->hardware_revision),
        (uint8_t*)dev_info_svc->hardware_revision);
    if(status) {
        FURI_LOG_E(TAG, "Failed to update firmware revision char: %d", status);
    }
    status = aci_gatt_update_char_value(
        dev_info_svc->service_handle,
        dev_info_svc->software_rev_char_handle,
        0,
        string_size(dev_info_svc->version_string),
        (uint8_t*)string_get_cstr(dev_info_svc->version_string));
    if(status) {
        FURI_LOG_E(TAG, "Failed to update software revision char: %d", status);
    }
    status = aci_gatt_update_char_value(
        dev_info_svc->service_handle,
        dev_info_svc->rpc_version_char_handle,
        0,
        strlen(dev_info_rpc_version),
        (uint8_t*)dev_info_rpc_version);
    if(status) {
        FURI_LOG_E(TAG, "Failed to update rpc version char: %d", status);
    }
}

void dev_info_svc_stop() {
    tBleStatus status;
    if(dev_info_svc) {
        string_clear(dev_info_svc->version_string);
        // Delete service characteristics
        status =
            aci_gatt_del_char(dev_info_svc->service_handle, dev_info_svc->man_name_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete manufacturer name char: %d", status);
        }
        status =
            aci_gatt_del_char(dev_info_svc->service_handle, dev_info_svc->serial_num_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete serial number char: %d", status);
        }
        status = aci_gatt_del_char(
            dev_info_svc->service_handle, dev_info_svc->firmware_rev_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete firmware revision char: %d", status);
        }
        status = aci_gatt_del_char(
            dev_info_svc->service_handle, dev_info_svc->software_rev_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete software revision char: %d", status);
        }
        status =
            aci_gatt_del_char(dev_info_svc->service_handle, dev_info_svc->rpc_version_char_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete rpc version char: %d", status);
        }
        // Delete service
        status = aci_gatt_del_service(dev_info_svc->service_handle);
        if(status) {
            FURI_LOG_E(TAG, "Failed to delete device info service: %d", status);
        }
        free(dev_info_svc);
        dev_info_svc = NULL;
    }
}

bool dev_info_svc_is_started() {
    return dev_info_svc != NULL;
}
