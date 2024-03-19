#include "dev_info_service.h"
#include "app_common.h"
#include <furi_ble/gatt.h>
#include <ble/ble.h>

#include <furi.h>
#include <protobuf_version.h>
#include <lib/toolbox/version.h>

#include "dev_info_service_uuid.inc"

#define TAG "BtDevInfoSvc"

typedef enum {
    DevInfoSvcGattCharacteristicMfgName = 0,
    DevInfoSvcGattCharacteristicSerial,
    DevInfoSvcGattCharacteristicFirmwareRev,
    DevInfoSvcGattCharacteristicSoftwareRev,
    DevInfoSvcGattCharacteristicRpcVersion,
    DevInfoSvcGattCharacteristicCount,
} DevInfoSvcGattCharacteristicId;

#define DEVICE_INFO_HARDWARE_REV_SIZE (4)
#define DEVICE_INFO_SOFTWARE_REV_SIZE (40)

struct BleServiceDevInfo {
    uint16_t service_handle;
    BleGattCharacteristicInstance characteristics[DevInfoSvcGattCharacteristicCount];
};

static const char dev_info_man_name[] = "Flipper Devices Inc.";
static const char dev_info_serial_num[] = "1.0";
static const char dev_info_rpc_version[] = TOSTRING(PROTOBUF_MAJOR_VERSION.PROTOBUF_MINOR_VERSION);
static char hardware_revision[DEVICE_INFO_HARDWARE_REV_SIZE] = {0};
static char software_revision[DEVICE_INFO_SOFTWARE_REV_SIZE] = {0};

static bool
    dev_info_char_data_callback(const void* context, const uint8_t** data, uint16_t* data_len) {
    *data_len = (uint16_t)strlen(context); //-V1029
    if(data) {
        *data = (const uint8_t*)context;
    }
    return false;
}

static const BleGattCharacteristicParams ble_svc_dev_info_chars[DevInfoSvcGattCharacteristicCount] =
    {[DevInfoSvcGattCharacteristicMfgName] =
         {.name = "Manufacturer Name",
          .data_prop_type = FlipperGattCharacteristicDataFixed,
          .data.fixed.length = sizeof(dev_info_man_name) - 1,
          .data.fixed.ptr = (const uint8_t*)&dev_info_man_name,
          .uuid.Char_UUID_16 = MANUFACTURER_NAME_UUID,
          .uuid_type = UUID_TYPE_16,
          .char_properties = CHAR_PROP_READ,
          .security_permissions = ATTR_PERMISSION_AUTHEN_READ,
          .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
          .is_variable = CHAR_VALUE_LEN_CONSTANT},
     [DevInfoSvcGattCharacteristicSerial] =
         {.name = "Serial Number",
          .data_prop_type = FlipperGattCharacteristicDataFixed,
          .data.fixed.length = sizeof(dev_info_serial_num) - 1,
          .data.fixed.ptr = (const uint8_t*)&dev_info_serial_num,
          .uuid.Char_UUID_16 = SERIAL_NUMBER_UUID,
          .uuid_type = UUID_TYPE_16,
          .char_properties = CHAR_PROP_READ,
          .security_permissions = ATTR_PERMISSION_AUTHEN_READ,
          .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
          .is_variable = CHAR_VALUE_LEN_CONSTANT},
     [DevInfoSvcGattCharacteristicFirmwareRev] =
         {.name = "Firmware Revision",
          .data_prop_type = FlipperGattCharacteristicDataCallback,
          .data.callback.context = hardware_revision,
          .data.callback.fn = dev_info_char_data_callback,
          .uuid.Char_UUID_16 = FIRMWARE_REVISION_UUID,
          .uuid_type = UUID_TYPE_16,
          .char_properties = CHAR_PROP_READ,
          .security_permissions = ATTR_PERMISSION_AUTHEN_READ,
          .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
          .is_variable = CHAR_VALUE_LEN_CONSTANT},
     [DevInfoSvcGattCharacteristicSoftwareRev] =
         {.name = "Software Revision",
          .data_prop_type = FlipperGattCharacteristicDataCallback,
          .data.callback.context = software_revision,
          .data.callback.fn = dev_info_char_data_callback,
          .uuid.Char_UUID_16 = SOFTWARE_REVISION_UUID,
          .uuid_type = UUID_TYPE_16,
          .char_properties = CHAR_PROP_READ,
          .security_permissions = ATTR_PERMISSION_AUTHEN_READ,
          .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
          .is_variable = CHAR_VALUE_LEN_CONSTANT},
     [DevInfoSvcGattCharacteristicRpcVersion] = {
         .name = "RPC Version",
         .data_prop_type = FlipperGattCharacteristicDataFixed,
         .data.fixed.length = sizeof(dev_info_rpc_version) - 1,
         .data.fixed.ptr = (const uint8_t*)&dev_info_rpc_version,
         .uuid.Char_UUID_128 = DEV_INVO_RPC_VERSION_UID,
         .uuid_type = UUID_TYPE_128,
         .char_properties = CHAR_PROP_READ,
         .security_permissions = ATTR_PERMISSION_AUTHEN_READ,
         .gatt_evt_mask = GATT_DONT_NOTIFY_EVENTS,
         .is_variable = CHAR_VALUE_LEN_CONSTANT}};

BleServiceDevInfo* ble_svc_dev_info_start(void) {
    BleServiceDevInfo* dev_info_svc = malloc(sizeof(BleServiceDevInfo));
    snprintf(
        software_revision,
        sizeof(software_revision),
        "%s %s %s %s",
        version_get_githash(NULL),
        version_get_gitbranch(NULL),
        version_get_gitbranchnum(NULL),
        version_get_builddate(NULL));
    snprintf(hardware_revision, sizeof(hardware_revision), "%d", version_get_target(NULL));

    // Add Device Information Service
    uint16_t uuid = DEVICE_INFORMATION_SERVICE_UUID;
    if(!ble_gatt_service_add(
           UUID_TYPE_16,
           (Service_UUID_t*)&uuid,
           PRIMARY_SERVICE,
           1 + 2 * DevInfoSvcGattCharacteristicCount,
           &dev_info_svc->service_handle)) {
        free(dev_info_svc);
        return NULL;
    }

    for(size_t i = 0; i < DevInfoSvcGattCharacteristicCount; i++) {
        ble_gatt_characteristic_init(
            dev_info_svc->service_handle,
            &ble_svc_dev_info_chars[i],
            &dev_info_svc->characteristics[i]);
        ble_gatt_characteristic_update(
            dev_info_svc->service_handle, &dev_info_svc->characteristics[i], NULL);
    }

    return dev_info_svc;
}

void ble_svc_dev_info_stop(BleServiceDevInfo* dev_info_svc) {
    furi_check(dev_info_svc);
    /* Delete service characteristics */
    for(size_t i = 0; i < DevInfoSvcGattCharacteristicCount; i++) {
        ble_gatt_characteristic_delete(
            dev_info_svc->service_handle, &dev_info_svc->characteristics[i]);
    }

    /* Delete service */
    ble_gatt_service_delete(dev_info_svc->service_handle);

    free(dev_info_svc);
}
